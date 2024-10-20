cmake_minimum_required(VERSION 3.27)
project(OS_Lab1_Client C)

set(CMAKE_C_STANDARD 11)

add_executable(OS_Lab1_Client Client.c)
target_link_libraries(OS_Lab1_Client PRIVATE ws2_32)
