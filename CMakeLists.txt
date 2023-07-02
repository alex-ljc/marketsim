cmake_minimum_required(VERSION 3.5)
# Project Name | Version Number | Language
project(marketsim VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# # Set the directories for header files and source files
# set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
# set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# # Add the include directory to the include path
# include_directories(${INCLUDE_DIR})

add_subdirectory(include)
add_subdirectory(src)
