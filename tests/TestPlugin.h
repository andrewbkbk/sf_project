// Developed by Andrey Bakhmutov, Sep 2018

#ifndef _TEST_PLUGIN_H_
#define _TEST_PLUGIN_H_

#include <atomic>
#include <string>
#include <cstdint>
#include <chrono>
#include <thread>
#include <sstream>

#include "Task.h"

class TestPlugin : public Task
{
public:
    TestPlugin(uint32_t duration_ms, bool status_code)
	: m_prc(0)
	, m_duration_ms(duration_ms)
	, m_status_code(status_code)
	, m_id_str("NOT_ON_THREAD")
	, m_started(false)
    {
    }
	         
    virtual ~TestPlugin() = default;

    bool run() override
    {
	m_started = true;
	std::thread::id this_id = std::this_thread::get_id();
	if(this_id != std::thread::id())
        {
      	    std::stringstream ss;
  	    ss << this_id;
	    m_id_str = ss.str();
        }
	std::this_thread::sleep_for(std::chrono::milliseconds(m_duration_ms));
	return m_status_code;
    }    
    int get_progress() override
    {
	if(!m_started) return 0;
	uint32_t prc = m_prc;
	m_prc += 10;
	if(m_prc > 100) m_prc = 100;
	return prc;
    }
    std::string get_status_msg() override
    {
	return m_id_str;
    }

private:
    std::atomic<uint32_t> m_prc;
    uint32_t m_duration_ms;
    bool m_status_code;
    std::string m_id_str;
    std::atomic<bool> m_started;
};

#endif
