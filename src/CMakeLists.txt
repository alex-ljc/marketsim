# Generates the penguins executable
add_executable(exchange
    exchange.cpp
)

target_link_libraries(exchange PRIVATE
    alex_library
)

target_compile_options(exchange PRIVATE -Wall -Wextra -O2 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wshift-overflow -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fno-sanitize-recover=undefined,integer -fstack-protector -Wno-unused-result -DL -g -Wc++11-extensions -std=c++20)

add_executable(tradingbot
    tradingbot.cpp
)

target_link_libraries(tradingbot PRIVATE
    alex_library
)

target_compile_options(tradingbot PRIVATE -Wall -Wextra -O2 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wshift-overflow -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fno-sanitize-recover=undefined,integer -fstack-protector -Wno-unused-result -DL -g -Wc++11-extensions -std=c++20)

add_executable(information
    information.cpp
)

target_link_libraries(information PRIVATE
    alex_library
)

target_compile_options(information PRIVATE -Wall -Wextra -O2 -Wshadow -Wformat=2 -Wfloat-equal -Wconversion -Wshift-overflow -Wcast-qual -Wcast-align -D_GLIBCXX_DEBUG -D_GLIBCXX_DEBUG_PEDANTIC -fno-sanitize-recover=undefined,integer -fstack-protector -Wno-unused-result -DL -g -Wc++11-extensions -std=c++20)