cmake_minimum_required(VERSION 3.5)
# Project Name | Version Number | Language
project(marketsim VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(MyExecutable LimitOrderBook.cpp)