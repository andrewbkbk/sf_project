#ifndef _PLUGIN_H_
#define _PLUGIN_H_

class Task;

struct PluginInfo
{
    int api_version; // version
    const char* api_name;
    Task* ( *create_task )( );
    void ( *delete_task )( Task* );
};

extern const PluginInfo plugin_info;

#endif
