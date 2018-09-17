#include "TaskPlugin.h"
#include "Task.h"
#include <iostream>

namespace
{

class MyTask : public Task
{
public:
    virtual ~MyTask( )
    {
        std::cout << "~MyTask" << std::endl;
    }
    virtual bool run( )
    {
        std::cout << "Run!!" << std::endl;
        return true;
    }
    virtual int get_progress( )
    {
        return 0;
    }
    virtual std::string get_status_msg( )
    {
        return 0;
    }
};

Task* create_task( )
{
    std::cout << "create_task() called" << std::endl;
    return new MyTask( );
}

void delete_task( Task* task )
{
    std::cout << "delete_task() called" << std::endl;
    delete task;
}
}

const PluginInfo plugin_info = {API_VERSION, API_NAME, create_task, delete_task};
