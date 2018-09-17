#ifndef _TASK_PROCESSOR_H_
#define _TASK_PROCESSOR_H_

#include <cstdint>
#include <memory>
#include <functional>

class Task;

/*
 * A base class for all Task Processors.
 * Implementations of this interface could provide different
 * processing strategies and scheduling policies.
 */

class TaskProcessor
{
public:
    // callback(task_id, is_success);
    using Callback = std::function< void( uint32_t, bool ) >;

    virtual ~TaskProcessor() = default;

    /**
     * Add a new task for processing. Once the task has been
     * processed, a callback 'cb' is called with the processing
     * status. In case the task cannot be added, the function
     * return false.
     */
    virtual bool add_task( std::shared_ptr< Task > task, uint32_t id,
			   Callback cb ) = 0;
};

#endif
