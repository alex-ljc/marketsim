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
# include_directories(${INCLUDE_DIRCMAKE_CXX_FLAGS})
set(Boost_INCLUDE_DIRS "/opt/homebrew/Cellar/boost/1.82.0_1/include")
set(Json_INCLUDE_DIRS "/opt/homebrew/Cellar/nlohmann-json/3.11.2/include")
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${Json_INCLUDE_DIRS})
add_subdirectory(include)
add_subdirectory(src)
target_compile_options(exchange PRIVATE -Wall -Wextra -O2 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wshift-overflow -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fno-sanitize-recover=undefined,integer -fstack-protector -Wno-unused-result -DL -g -Wc++11-extensions -std=c++20)
