cmake_minimum_required(VERSION 3.1)
project(BandwidthTester)

set(CMAKE_CXX_STANDARD 11)
find_package(Threads)

set(SOURCE_DIR "..")

set(SOURCES ${SOURCE_DIR}/TaskManager.cpp
            ${SOURCE_DIR}/TaskManager.h
            ${SOURCE_DIR}/TaskProcessor.h
            ${SOURCE_DIR}/TaskProcessorAdaptiveThreads.cpp
            ${SOURCE_DIR}/TaskProcessorAdaptiveThreads.h
	    ${SOURCE_DIR}/TaskPlugin.h)

add_library(plugin_wrong_API SHARED TestPlugin_wrong_API.cpp)
target_include_directories(plugin_wrong_API PRIVATE "..")
	  
add_library(plugin_1000ms_false SHARED TestPlugin_1000ms_false.cpp)
target_include_directories(plugin_1000ms_false PRIVATE "..")

add_library(plugin_1000ms_true SHARED TestPlugin_1000ms_true.cpp)
target_include_directories(plugin_1000ms_true PRIVATE "..")

add_library(plugin_1001ms_false SHARED TestPlugin_1001ms_false.cpp)
target_include_directories(plugin_1001ms_false PRIVATE "..")

add_library(plugin_1001ms_true SHARED TestPlugin_1001ms_true.cpp)
target_include_directories(plugin_1001ms_true PRIVATE "..")

add_library(plugin_10ms_false SHARED TestPlugin_10ms_false.cpp)
target_include_directories(plugin_10ms_false PRIVATE "..")

add_library(plugin_10ms_true SHARED TestPlugin_10ms_true.cpp)
target_include_directories(plugin_10ms_true PRIVATE "..")

add_library(plugin_1500ms_true SHARED TestPlugin_1500ms_true.cpp)
target_include_directories(plugin_1500ms_true PRIVATE "..")

add_library(plugin_1ms_false SHARED TestPlugin_1ms_false.cpp)
target_include_directories(plugin_1ms_false PRIVATE "..")

add_library(plugin_1ms_true SHARED TestPlugin_1ms_true.cpp)
target_include_directories(plugin_1ms_true PRIVATE "..")

add_library(plugin_2000ms_false SHARED TestPlugin_2000ms_false.cpp)
target_include_directories(plugin_2000ms_false PRIVATE "..")

add_library(plugin_2000ms_true SHARED TestPlugin_2000ms_true.cpp)
target_include_directories(plugin_2000ms_true PRIVATE "..")

add_library(plugin_2001ms_true SHARED TestPlugin_2001ms_true.cpp)
target_include_directories(plugin_2001ms_true PRIVATE "..")

add_library(plugin_2002ms_true SHARED TestPlugin_2002ms_true.cpp)
target_include_directories(plugin_2002ms_true PRIVATE "..")

add_library(plugin_2100ms_true SHARED TestPlugin_2100ms_true.cpp)
target_include_directories(plugin_2100ms_true PRIVATE "..")

add_library(plugin_2200ms_true SHARED TestPlugin_2200ms_true.cpp)
target_include_directories(plugin_2200ms_true PRIVATE "..")

add_library(plugin_2300ms_true SHARED TestPlugin_2300ms_true.cpp)
target_include_directories(plugin_2300ms_true PRIVATE "..")

add_library(plugin_9000ms_false SHARED TestPlugin_9000ms_false.cpp)
target_include_directories(plugin_9000ms_false PRIVATE "..")

add_library(plugin_9000ms_true SHARED TestPlugin_9000ms_true.cpp)
target_include_directories(plugin_9000ms_true PRIVATE "..")

add_executable(test_loader test_loader.cpp ${SOURCES})
target_include_directories(test_loader PRIVATE "..")
target_link_libraries(test_loader dl ${CMAKE_THREAD_LIBS_INIT})
