cmake_minimum_required(VERSION 3.22)
project(os_ex2)

set(CMAKE_CXX_STANDARD 11)

include_directories(.)

add_executable(os_ex2
        uthreads.cpp
        uthreads.h
        Thread.cpp
        Thread.h
        ThreadManager.cpp
        ThreadManager.h
        main.cpp)
