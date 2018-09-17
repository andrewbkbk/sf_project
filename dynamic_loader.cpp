// Developer by Andrey Bakhmutov, Sep 2018

#include <iostream>

#include "TaskManager.h"
#include "TaskProcessorAdaptiveThreads.h"

bool parse_args( int argc, char* argv[], TaskManager::Config& config )
{
    if ( argc != 2 )
    {
        std::cout << "Usage: " << argv[0] << " plugin_dir" << std::endl;
        return false;
    }

    config.plugins_dir = argv[1];
    config.disable_api_version_check = true;

    return true;
}

int main( int argc, char* argv[] )
{
    TaskManager::Config tm_config;

    if ( !parse_args( argc, argv, tm_config ) )
        return -1;

    TaskProcessorAdaptiveThreads::Config tp_config;
    TaskProcessorAdaptiveThreads tp( tp_config );

    TaskManager( tm_config, tp ).run( );

    return 0;
}
