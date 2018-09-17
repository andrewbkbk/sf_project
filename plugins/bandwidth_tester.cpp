// Developed by Andrey Bakhmutov, Sep 2018

#include <iomanip>
#include <iostream>
#include <thread>
#include <unistd.h>

#include "BandwidthTester.h"

// This file is used to test BandwidthTester functionality

std::atomic< bool > running{false};

void run_task( Task& task )
{
    if ( !task.run( ) )
    {
        std::cout << "cannot start task" << std::endl;
    }

    running = false;
}

const std::string server_name = "speedtest.wdc01.softlayer.com";
const uint16_t port = 80;
const std::string path = "/downloads/test100.zip";

int poll_interval_sec = 2;

bool parse_args( int argc, char* argv[] )
{
    if ( argc != 1 && argc != 2 )
    {
        std::cout << "Usage: " << argv[0] << " [poll_interval_seconds]" << std::endl;
        return false;
    }
    if ( argc == 2 )
    {
        int interval_sec = std::stoi( ( argv[1] ) );
        if ( interval_sec > 0 )
            poll_interval_sec = interval_sec;
    }
    return true;
}

int main( int argc, char* argv[] )
{
    if ( !parse_args( argc, argv ) )
        return -1;

    BandwidthTester tester( server_name, port, path );

    running = true;

    std::thread t( run_task, std::ref( tester ) );

    int prc = 0;
    do
    {
        prc = tester.get_progress( );
        std::string status = tester.get_status_msg( );

        std::cout << "[" << std::setw( 3 ) << prc << "%"
                  << "] " << status << std::endl;
        sleep( poll_interval_sec );
    } while ( running.load( ) && prc < 100 );

    if ( t.joinable( ) )
        t.join( );

    return 0;
}
