cmake_minimum_required(VERSION 3.28)
project(CGC C)

set(CMAKE_C_STANDARD 23)

add_executable(CGC main.c
        cgc.c
        cgc.h)
