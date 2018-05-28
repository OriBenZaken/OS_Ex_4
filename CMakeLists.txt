cmake_minimum_required(VERSION 3.10)
project(OS_Ex_4 C)

set(CMAKE_C_STANDARD 90)
SET(CMAKE_C_FLAGS -pthread)

add_executable(OS_Ex_4 main.c osqueue.c osqueue.h threadPool.h threadPool.c)