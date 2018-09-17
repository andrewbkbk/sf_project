// Developed by Andrey Bakhmutov, Sep 2018

#ifndef _TASK_PROCESSOR_ADAPTIVE_THREADS_H_
#define _TASK_PROCESSOR_ADAPTIVE_THREADS_H_

#include "TaskProcessor.h"

#include <atomic>
#include <queue>
#include <string>
#include <thread>
#include <condition_variable>
#include <mutex>

/**
 * This implementation of the TaskProcessor interface provides an
 * adaptive thread management policy:
 *
 * The class maintains a pool of threads which size cannot exceed a
 * predefined limit. Threads are created on demand when a new task is
 * added: if there are no idle threads waiting for tasks and the
 * number of threads is below the limit, a new thread is created,
 * otherwise the task is added to the queue to be processed later.
 * If a thread is idle for a long time (predefined parameter), it
 * termitates freeing allocated resources.
 */

class Task;

class TaskProcessorAdaptiveThreads : public TaskProcessor
{
public:

    struct Config
    {
	// maximum number of threads in the pool
        size_t max_threads = 10;
	// the time a thread is allowed to be idle
        uint32_t thread_waiting_time_sec = 60;
    };

    TaskProcessorAdaptiveThreads( const Config& config );
    ~TaskProcessorAdaptiveThreads( ) override;

    bool add_task( std::shared_ptr< Task > task, uint32_t id, Callback cb ) override;

private:
    void run( );
    void provide_thread( );

    struct TaskInfo
    {
        std::shared_ptr< Task > task;
        uint32_t id;
        Callback cb;
    };

private:
    Config m_config;
    std::queue< TaskInfo > m_waiting_queue;
    std::atomic< bool > m_running;
    int m_busy_threads;
    std::vector< std::thread > m_threads_pool;
    std::vector< std::thread::id > m_exited_threads;
    std::mutex m_mutex;
    std::condition_variable m_cv;
};

#endif
