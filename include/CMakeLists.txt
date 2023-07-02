
add_library(alex_library STATIC LimitOrderBook.cpp LimitOrderBook.h)

target_include_directories(alex_library INTERFACE ${CMAKE_CURRENT_SOURCE_DIR})