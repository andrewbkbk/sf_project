// Developed by Andrey Bakhmutov, Sep 2018

#include "TaskProcessorAdaptiveThreads.h"
#include "Task.h"

#include <algorithm>
#include <assert.h>

#ifdef _DEBUG
#include <iostream>
#endif

TaskProcessorAdaptiveThreads::TaskProcessorAdaptiveThreads( const Config& config )
    : m_config( config )
    , m_running( true )
    , m_busy_threads( 0 )
{
}

TaskProcessorAdaptiveThreads::~TaskProcessorAdaptiveThreads( )
{
    m_running = false;

    for ( auto& t : m_threads_pool )
    {
        if ( t.joinable( ) )
        {
  	    t.join( );
	}
    }
}

bool TaskProcessorAdaptiveThreads::add_task( std::shared_ptr< Task > task, uint32_t id,
					     Callback cb )
{
    if ( !m_running )
        return false;

    {
        std::lock_guard< std::mutex > lock( m_mutex );

        m_waiting_queue.push( {task, id, cb} );
        provide_thread( );
    }

    m_cv.notify_one( );

    return true;
}

void TaskProcessorAdaptiveThreads::provide_thread( )
{
    // the number of threads available for this task
    int n_free_threads = m_threads_pool.size( ) - m_exited_threads.size( ) -
	m_busy_threads - m_waiting_queue.size( );

#ifdef _DEBUG
    std::cout << "adding task, threads available: " << n_free_threads << std::endl;
#endif

    if ( n_free_threads < 0 )
    {
        // no available threads in the waiting state found
        if ( !m_exited_threads.empty( ) )
        {
            // reuse one of the exited thread
            std::thread::id id = m_exited_threads.back( );
            m_exited_threads.pop_back( );
            auto it = std::find_if( m_threads_pool.begin( ), m_threads_pool.end( ),
                                    [id]( const std::thread& t ) {
					return t.get_id( ) == id; } );
            assert( it != m_threads_pool.cend( ) );
            if ( it != m_threads_pool.cend( ) )
            {
                if ( it->joinable( ) )
  	        {
		    it->join( );
		}
                *it = std::thread( &TaskProcessorAdaptiveThreads::run, this );
#ifdef _DEBUG
                std::cout << "thread reused, id = " << id << std::endl;
#endif
            }
        }
        else if ( m_threads_pool.size( ) < m_config.max_threads )
        {
            // create a new thread
            m_threads_pool.push_back( std::thread( &TaskProcessorAdaptiveThreads::run,
						   this ) );
#ifdef _DEBUG
            std::cout << "a new thread created, id = " << m_threads_pool.back( ).get_id( )
                      << std::endl;
#endif
        }
        else
        {
        // no thread available for the task
        // the task will wait in a queue until one of the threads
        // become available
#ifdef _DEBUG
            std::cout << "no available threads found, delay execution" << std::endl;
#endif
        }
    }
}

void TaskProcessorAdaptiveThreads::run( )
{
    const auto sec = std::chrono::seconds( m_config.thread_waiting_time_sec );

    while ( m_running )
    {
        TaskInfo task_info;
        {
            std::unique_lock< std::mutex > lk( m_mutex );

            if ( !m_cv.wait_for( lk, sec, [this] { return !m_waiting_queue.empty( ); } ) )
            {
#ifdef _DEBUG
                std::cout << "exiting from an idle thread" << std::endl;
#endif
                break;
            }

            task_info = m_waiting_queue.front( );
            m_waiting_queue.pop( );
            m_busy_threads++;
        }

        bool status = task_info.task->run( );
        task_info.cb( task_info.id, status );
    }

    m_exited_threads.push_back( std::this_thread::get_id( ) );
}
