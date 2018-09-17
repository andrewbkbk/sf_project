#include <iostream>

#include "TaskManager.h"
#include "TaskProcessorAdaptiveThreads.h"

uint32_t program_run_sec = 0;

size_t max_threads = 0;
uint32_t thread_waiting_time_sec = 0;

bool parse_args(int argc, char *argv[])
{
    if(argc != 2 && argc != 3 && argc != 4)
    {
	std::cout << "Usage: " << argv[0] << " time_sec [max_threads [thread_wait_sec]]" << std::endl;
	return false;
    }

    program_run_sec = std::stoi(argv[1]);

    if(argc >= 3) max_threads = std::stoi(argv[2]);
    if(argc == 4) thread_waiting_time_sec = std::stoi(argv[3]);

    return true;
}

int main(int argc, char *argv[])
{
    if(!parse_args(argc, argv)) return -1;

    TaskManager::Config tm_config;

    // default configuration for the Task Manager
    tm_config.plugins_dir = "./temp_test_plugins";
    tm_config.poll_task_interval_sec = 1u;
    tm_config.scan_plugins_dir_interval_sec = 1u;

    // configuration for the Task Processor
    TaskProcessorAdaptiveThreads::Config tp_config;
    tp_config.max_threads = max_threads;
    tp_config.thread_waiting_time_sec = thread_waiting_time_sec;

    std::thread t([](){
         std::this_thread::sleep_for(std::chrono::seconds(program_run_sec));
	 exit(14);
	});

    TaskProcessorAdaptiveThreads tp(tp_config);
        
    TaskManager(tm_config, tp).run();

    if(t.joinable()) t.join();

    return 0;
}
