cmake_minimum_required(VERSION 3.1)
project(BandwidthTester)

set(CMAKE_CXX_STANDARD 11)
find_package(Threads)

set(SOURCES TaskManager.cpp
            TaskManager.h
            TaskProcessor.h
            TaskProcessorAdaptiveThreads.cpp
	    TaskProcessorAdaptiveThreads.h
	    TaskPlugin.cpp
	    TaskPlugin.h)

add_executable(dynamic-loader dynamic_loader.cpp ${SOURCES})
target_include_directories(dynamic-loader PRIVATE ".")
target_link_libraries(dynamic-loader dl ${CMAKE_THREAD_LIBS_INIT})

add_subdirectory(plugins)
add_subdirectory(tests)
