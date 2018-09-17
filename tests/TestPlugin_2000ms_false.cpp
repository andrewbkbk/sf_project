// Developed by Andrey Bakhmutov, Sep 2018

#include "TaskPlugin.h"
#include "TestPlugin.h"

namespace
{
constexpr uint32_t DURATION_MS = 2000;
constexpr bool STATUS_CODE = false;
}

//////////// Plugin interface implementation ////////////

Task *create_task()
{
    return new TestPlugin(DURATION_MS, STATUS_CODE);
}

void delete_task(Task *task)
{
    delete task;
}

const PluginInfo plugin_info = { API_VERSION,
				 API_NAME,
				 create_task,
				 delete_task };

/////////////////////////////////////////////////////////
