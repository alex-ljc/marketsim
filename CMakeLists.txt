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
set(ROOT_INCLUDE_DIRS "/opt/homebrew/Cellar/root/6.28.04/include")
include_directories(SYSTEM ${Boost_INCLUDE_DIRS} ${Json_INCLUDE_DIRS})
add_subdirectory(include)
add_subdirectory(src)

# Set the CMP0135 policy to NEW
cmake_policy(SET CMP0135 NEW)

include(FetchContent)
FetchContent_Declare(
  googletest
  URL https://github.com/google/googletest/archive/03597a01ee50ed33e9dfd640b249b4be3799d395.zip
)
# For Windows: Prevent overriding the parent project's compiler/linker settings
set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

