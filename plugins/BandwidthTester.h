// Developed by Andrey Bakhmutov, Sep 2018

#ifndef _BANDWIDTH_TESTER_H_
#define _BANDWIDTH_TESTER_H_

#include <atomic>
#include <chrono>
#include <cstdint>
#include <mutex>
#include <string>

#include "Task.h"
#include "TaskPlugin.h"

class BandwidthTester : public Task
{
public:
    BandwidthTester( const std::string& server_name, uint16_t port, const std::string& path );
    BandwidthTester( const std::string& url );

    virtual ~BandwidthTester( ) = default;

    bool run( ) override;
    int get_progress( ) override;
    std::string get_status_msg( ) override;

private:
    void reset_stat( );
    void adjust_stat( size_t bytes_read );

private:
    std::atomic< bool > m_progress;

    const std::string m_server_name;
    uint16_t m_port;
    const std::string m_path;

    std::mutex m_mutex;
    size_t m_size_dowloaded; // total size downloaded
    size_t m_content_length; // payload size from HTTP response
    size_t m_dowloaded_delta;
    std::chrono::high_resolution_clock::time_point m_start_delta;
    double m_speed_kbs; // download speed for the last time slot
};

#endif
