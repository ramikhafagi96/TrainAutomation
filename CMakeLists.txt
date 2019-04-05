cmake_minimum_required(VERSION 3.12)
project(TrainStation C)

set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -std=c99 -pthread")

add_executable(TrainStation train.c)