cmake_minimum_required(VERSION 3.5)
# Project Name | Version Number | Language
project(marketsim VERSION 0.0.1 LANGUAGES CXX)

# Set C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Set the directories for header files and source files
set(INCLUDE_DIR ${CMAKE_CURRENT_SOURCE_DIR}/include)
set(SRC_DIR ${CMAKE_CURRENT_SOURCE_DIR}/src)

# Add the include directory to the include path
include_directories(${INCLUDE_DIR})

# Collect all source files in the source directory
file(GLOB SOURCES ${SRC_DIR}/*.cpp)

# Create an executable from the source files
add_executable(marketsim ${SOURCES})

# Set the target properties, if needed
target_compile_options(marketsim PRIVATE -Wall -Wextra -O2 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wshift-overflow -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fsanitize=address -fsanitize=undefined -fno-sanitize-recover=undefined,integer -fstack-protector -Wno-unused-result -DL -g -Wc++11-extensions -std=c++20)

# Optional: If you have additional libraries, add them here
# target_link_libraries(MyProject library1 library2 ...)

# Optional: If you have additional dependencies, find and link them here
# find_package(Boost REQUIRED COMPONENTS ... )
# include_directories(${Boost_INCLUDE_DIRS})
# target_link_libraries(MyProject ${Boost_LIBRARIES})
