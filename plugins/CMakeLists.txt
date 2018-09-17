cmake_minimum_required(VERSION 3.1)
project(BandwidthTester)

set(CMAKE_CXX_STANDARD 11)
find_package(Threads)

set(SOURCES BandwidthTester.cpp
            BandwidthTester.h
	    NetworkClient.cpp
	    NetworkClient.h
	    SimpleHttpClient.cpp
	    SimpleHttpClient.h)

add_library(bandwidth_prober SHARED ${SOURCES})
target_include_directories(bandwidth_prober PRIVATE "..")
target_link_libraries (bandwidth_prober  ${CMAKE_THREAD_LIBS_INIT})

add_executable(bandwidth_tester bandwidth_tester.cpp ${SOURCES})
target_include_directories(bandwidth_tester PRIVATE "..")
target_link_libraries (bandwidth_tester  ${CMAKE_THREAD_LIBS_INIT})
