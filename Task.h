#ifndef _TASK_H_
#define _TASK_H_

#include <string>

constexpr int API_VERSION = 1;
constexpr const char* API_NAME = "Task";

class Task
{
public:
    virtual ~Task( )
    {
    }
    virtual bool run( ) = 0;
    virtual int get_progress( ) = 0;
    virtual std::string get_status_msg( ) = 0;
};

#endif
