cmake_minimum_required(VERSION 3.31)
project(prog3_word_counter_2025_1)

set(CMAKE_CXX_STANDARD 20)

add_executable(prog3_word_counter_2025_1 main.cpp
        word_counter_aggregator.cpp
        word_counter_aggregator.h)
