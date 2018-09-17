// Developer by Andrey Bakhmutov, Sep 2018

#ifndef _TASK_MANAGER_H_
#define _TASK_MANAGER_H_

#include <atomic>
#include <cstdint>
#include <list>
#include <memory>
#include <mutex>
#include <string>
#include <unordered_set>

class Task;
class TaskProcessor;
class PluginInfo;

/**
 * The class provides tasks and plugins management. It periodically
 * scans a plugin directory adding and removing plugins to/from the
 * system. Once a new plugin is found, a corresponding task is added
 * for processing using a TaskProcessor interface. If a plugin is
 * removed from the directory, it is marked as deleted and will be
 * closed as soon as its task finishes.
 * The class also scans active tasks reporting their progress and
 * status.
 */

class TaskManager
{
public:
    struct Config
    {
	// a directory for plugin libraries
        std::string plugins_dir;
	
	// if enabled, plugin's API version and name will not be checked
	bool disable_api_version_check = false;

	// time interval to report active tasks progress
        uint32_t poll_task_interval_sec = 5u;

	// time interval to scan the plugin directory
        uint32_t scan_plugins_dir_interval_sec = 8u;
    };

    struct Plugin
    {
        enum class TaskStatus : uint8_t
        {
            PROGRESS,
            SUCCESS,
            FAILED,
            PROCESSED
        };

        void* handle; // plugin handle

        std::string name; // plugin name

        PluginInfo* info;

        // the plugin's library is no longer in the directory;
        // the plugin must be unloaded after its active task finishes
        bool unload;

        uint32_t task_id; // unique id to identify an active task
        TaskStatus task_status;

        // a pointer to the plugin's active task being executed by
        // the TaskProcessor
        std::weak_ptr< Task > task;
    };

    TaskManager( const Config& config, TaskProcessor& task_processor );
    ~TaskManager( );

    bool run( ); // run the manager

    // this callback will be called by Task Processor when a task finishes
    void on_task_finished( uint32_t id, bool is_success );

private:
    bool scan_plugin_dir( std::unordered_set< std::string >& entries );
    void process_plugins( std::unordered_set< std::string >& entries );
    void poll_active_tasks( );

    bool open_plugin( const std::string& name, Plugin& plugin );
    void close_plugin( Plugin& plugin );

    void print_status( const std::string& name, int prc, const std::string& msg );
    void print_status( const std::string& name, Plugin::TaskStatus status );

    void log( const std::string& msg );

private:
    Config m_config;
    TaskProcessor& m_task_processor;
    std::list< Plugin > m_plugins;
    std::atomic< bool > m_running;
    uint32_t m_id;
    std::mutex m_mutex;
};

#endif
