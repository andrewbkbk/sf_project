// Developer by Andrey Bakhmutov, Sep 2018

#include <dirent.h>
#include <dlfcn.h>
#include <iomanip>
#include <iostream>
#include <sstream>
#include <thread>
#include <string.h>

#include "Task.h"
#include "TaskManager.h"
#include "TaskPlugin.h"
#include "TaskProcessor.h"


TaskManager::TaskManager( const Config& config, TaskProcessor& task_processor )
    : m_config( config )
    , m_task_processor( task_processor )
    , m_running( false )
    , m_id( 0 )
{
}

TaskManager::~TaskManager( )
{
    // TODO// synchronise with the running tasks and close plugins
}

bool TaskManager::run( )
{
    if ( !m_config.scan_plugins_dir_interval_sec || !m_config.poll_task_interval_sec )
    {
        return false;
    }

    m_running = true;

    uint32_t counter = 0;

    while ( m_running )
    {
        if ( !( counter % m_config.scan_plugins_dir_interval_sec ) )
        {
            std::unordered_set< std::string > entries;

            // create new tasks and mark existing plugins as expired
            if ( scan_plugin_dir( entries ) )
                process_plugins( entries );
        }

        if ( !( counter % m_config.poll_task_interval_sec ) )
        {
            poll_active_tasks( );
        }

        std::this_thread::sleep_for( std::chrono::seconds( 1 ) );
        counter++;
    }

    return true;
}

bool TaskManager::scan_plugin_dir( std::unordered_set< std::string >& entries )
{
    if ( m_config.plugins_dir.empty( ) )
        return false;

    DIR* dir = opendir( m_config.plugins_dir.c_str( ) );

    if ( !dir )
        return false;

    dirent* ent;

    while ( ( ent = readdir( dir ) ) != 0 )
    {
        if ( ent->d_type != DT_REG )
            continue;
        entries.insert( std::string( ent->d_name ) );
    }

    closedir( dir );

#ifdef _DEBUG
    std::cout << "plugin directory read, entires: " << entries.size( ) << std::endl;
#endif

    return true;
}

void TaskManager::process_plugins( std::unordered_set< std::string >& entries )
{
    std::lock_guard< std::mutex > lock( m_mutex );

    for ( auto& plugin : m_plugins )
    {
        auto it = entries.find( plugin.name );
        if ( it != entries.end( ) )
        {
            if ( plugin.unload )
                plugin.unload = false;
            entries.erase( it );
        }
        else
        {
            plugin.unload = true;
#ifdef _DEBUG
            std::cout << "plugin removed, mark it as expired, " << plugin.name
		      << std::endl;
#endif
        }
    }

    for ( auto it = m_plugins.begin( ); it != m_plugins.end( ); )
    {
        auto task = it->task.lock( );

        if ( !task && it->unload && it->task_status == Plugin::TaskStatus::PROCESSED )
        {
        // plugin has been deleted from the directory and not active;
        // close the library
#ifdef _DEBUG
            std::cout << "plugin unloaded, " << it->name << std::endl;
#endif
            close_plugin( *it );
            it = m_plugins.erase( it );
        }
        else
            ++it;
    }

    for ( const auto& entry : entries )
    {
        Plugin plugin;

        if ( !open_plugin( entry, plugin ) )
        {
#ifdef _DEBUG
            std::cout << "failed to open plugin, " << entry << std::endl;
#endif
            continue;
        }

        // a new plugin has been detected, schedule a new task

        plugin.task_id = ++m_id;
        plugin.task_status = Plugin::TaskStatus::PROGRESS;

        std::shared_ptr< Task > task( plugin.info->create_task( ),
				      plugin.info->delete_task );

        plugin.task = task;

        auto cb
            = [this]( uint32_t id, bool is_success ) {
	    this->on_task_finished( id, is_success ); };

        m_task_processor.add_task( task, plugin.task_id, cb );

        m_plugins.push_back( plugin );

#ifdef _DEBUG
        std::cout << "task created, id = " << plugin.task_id << ", name = "
		  << plugin.name << std::endl;
#endif
    }
}

void TaskManager::poll_active_tasks( )
{
    std::lock_guard< std::mutex > lock( m_mutex );

    for ( auto it = m_plugins.begin( ); it != m_plugins.end( ); ++it )
    {
        auto task = it->task.lock( );

        if ( task )
        {
            // the task is active, print out its progress
            int prc = task->get_progress( );
            std::string status_msg = task->get_status_msg( );
            print_status( it->name, prc, status_msg );
        }
        else if ( it->task_status == Plugin::TaskStatus::SUCCESS
                  || it->task_status == Plugin::TaskStatus::FAILED )
        {
            // the task has been finished, print the status
            print_status( it->name, it->task_status );
            it->task_status = Plugin::TaskStatus::PROCESSED;
        }
    }
}

void TaskManager::print_status( const std::string& name, int prc, const std::string& msg )
{
    std::stringstream ss;
    ss << "[" << std::setw( 29 ) << name << "] Progress: " << std::setw( 3 ) << prc
       << "%, Status: " << msg;
    log( ss.str( ) );
}

void TaskManager::print_status( const std::string& name, Plugin::TaskStatus status )
{
    const char* s = status == Plugin::TaskStatus::SUCCESS ? "SUCCESS" : "ERROR";
    std::stringstream ss;
    ss << "[" << std::setw( 29 ) << name << "] " << s;
    log( ss.str( ) );
}

void TaskManager::on_task_finished( uint32_t id, bool is_success )
{
    std::lock_guard< std::mutex > lock( m_mutex );

    for ( auto& plugin : m_plugins )
    {
        if ( plugin.task_id == id )
        {
            plugin.task_status
                = is_success ? Plugin::TaskStatus::SUCCESS : Plugin::TaskStatus::FAILED;
            break;
        }
    }
}

bool TaskManager::open_plugin( const std::string& name, Plugin& plugin )
{
    std::string lib_path = m_config.plugins_dir + "/" + name;

    void* handle = dlopen( lib_path.c_str( ), RTLD_LAZY );

    if ( !handle )
        return false;

    dlerror( );

    PluginInfo* info = ( PluginInfo* )dlsym( handle, "plugin_info" );

    if ( !info )
        return false;

    if ( !m_config.disable_api_version_check )
    {
        // check API version
        if ( info->api_version != API_VERSION )
            return false;

        // check API name
        if ( strcmp( info->api_name, API_NAME ) )
            return false;
    }

    plugin.handle = handle;
    plugin.name = name;
    plugin.info = info;
    plugin.unload = false;

    return true;
}

void TaskManager::close_plugin( Plugin& plugin )
{
    dlclose( plugin.handle );
}

void TaskManager::log( const std::string& msg )
{
    auto now = std::chrono::system_clock::now( );
    time_t now_tt = std::chrono::system_clock::to_time_t( now );

    tm ts;
    localtime_r( ( const time_t* )&now_tt, &ts );

    char buf[32];
    snprintf( buf, sizeof( buf ), "[%04d-%02d-%02d %02d:%02d:%02d]", 1900 + ts.tm_year,
              ts.tm_mon + 1, ts.tm_mday, ts.tm_hour, ts.tm_min, ts.tm_sec );

    std::cout << buf << msg << std::endl;
}
