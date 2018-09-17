// Developed by Andrey Bakhmutov, Sep 2018

#include <cstdio>

#include "BandwidthTester.h"
#include "SimpleHttpClient.h"

#ifdef _DEBUG
#include <iostream>
#endif

namespace
{
constexpr uint32_t STAT_INTERVAL_SEC = 3;
constexpr uint32_t MAX_TIMEOUTS = 2; // max number of consecutive timeouts
}

BandwidthTester::BandwidthTester( const std::string& server_name,
                                  uint16_t port,
                                  const std::string& path )
    : m_progress( false )
    , m_server_name( server_name )
    , m_port( port )
    , m_path( path )
{
    reset_stat( );
}

BandwidthTester::BandwidthTester( const std::string& url )
{
    // TODO// not implemented yet
}

void BandwidthTester::reset_stat( )
{
    std::lock_guard< std::mutex > lock( m_mutex );

    m_dowloaded_delta = 0;
    m_speed_kbs = 0.0;
    m_start_delta = std::chrono::high_resolution_clock::now( );

    m_size_dowloaded = 0;
    m_content_length = 0;
    m_speed_kbs = 0;
}

bool BandwidthTester::run( )
{
    bool expected = false;

    if ( !m_progress.compare_exchange_strong( expected, true ) )
    {
        return false;
    }

    reset_stat( );

    SimpleHttpClient http_client;
    if ( !http_client.open( m_server_name.c_str( ), m_port ) )
    {
#ifdef _DEBUG
        std::cout << "cannot connect, " << m_server_name << ", port = " << m_port << std::endl;
#endif
        return false;
    }

    size_t content_length;
    uint32_t http_code;
    SimpleHttpClient::Status status;

    status = http_client.send_get_request( m_path, content_length, http_code );

    if ( status != SimpleHttpClient::Status::OK )
    {
#ifdef _DEBUG
        std::cout << "GET request failed, status: " << SimpleHttpClient::status2str( status )
                  << ", http: " << http_code << std::endl;
#endif
        return false;
    }

    {
        std::lock_guard< std::mutex > lock( m_mutex );
        m_start_delta = std::chrono::high_resolution_clock::now( );
        m_content_length = content_length;
    }

    uint32_t num_timeouts = 0; // number of consecutive timeouts

    while ( 1 )
    {
        char buf[2048];
        size_t bytes_read = 0;

        status = http_client.recv_payload_chunk( buf, sizeof( buf ), bytes_read );

        if ( status == SimpleHttpClient::Status::Timeout )
            num_timeouts++;
        else
            num_timeouts = 0;

        if ( status != SimpleHttpClient::Status::OK )
        {
            if ( status != SimpleHttpClient::Status::Timeout || num_timeouts == MAX_TIMEOUTS )
            {
#ifdef _DEBUG
                std::cout << "dowloading failed, status: " << SimpleHttpClient::status2str( status )
                          << std::endl;
#endif
                return false;
            }
        }

        adjust_stat( bytes_read );

        if ( status != SimpleHttpClient::Status::Timeout && !bytes_read )
            break;

        {
            std::lock_guard< std::mutex > lock( m_mutex );
            if ( m_size_dowloaded >= m_content_length )
                break;
        }
    }

    m_progress = false;

    return true;
}

void BandwidthTester::adjust_stat( size_t bytes_read )
{
    std::lock_guard< std::mutex > lock( m_mutex );

    auto time_now = std::chrono::high_resolution_clock::now( );
    auto elapsed_mks
        = std::chrono::duration_cast< std::chrono::microseconds >( time_now - m_start_delta );

    m_size_dowloaded += bytes_read;
    m_dowloaded_delta += bytes_read;

    if ( elapsed_mks.count( ) / 1000000 > 2 * STAT_INTERVAL_SEC )
    {
        // too much time elapsed, the old value is not valid
        m_speed_kbs = 0;
        m_start_delta = time_now;
    }
    else if ( elapsed_mks.count( ) / 1000000 >= STAT_INTERVAL_SEC )
    {
        m_speed_kbs = ( m_dowloaded_delta * 1000000 / elapsed_mks.count( ) ) / 1024;
        m_dowloaded_delta = 0;
        m_start_delta = time_now;
    }
}

int BandwidthTester::get_progress( )
{
    std::lock_guard< std::mutex > lock( m_mutex );

    if ( !m_content_length )
        return 0;
    if ( m_size_dowloaded >= m_content_length )
        return 100;

    return ( m_size_dowloaded * 100 ) / m_content_length;
}

// E.g: 30KB/s
std::string BandwidthTester::get_status_msg( )
{
    std::lock_guard< std::mutex > lock( m_mutex );

    char buf[32];
    if ( m_speed_kbs < 10 )
        snprintf( buf, sizeof( buf ), "%.1lfKB/s", m_speed_kbs );
    else
        snprintf( buf, sizeof( buf ), "%.0lfKB/s", m_speed_kbs );

    return std::string( buf );
}

//////////// Plugin interface implementation ////////////

Task* create_task( )
{
    // TODO// read these parameters from an optional configuration file
    static const std::string server_name = "speedtest.wdc01.softlayer.com";
    static const uint16_t port = 80;
    static const std::string path = "/downloads/test100.zip";

    return new BandwidthTester( server_name, port, path );
}

void delete_task( Task* task )
{
    delete task;
}

const PluginInfo plugin_info = {API_VERSION, API_NAME, create_task, delete_task};

/////////////////////////////////////////////////////////
