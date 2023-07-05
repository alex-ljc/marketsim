# Generates the penguins executable
add_executable(exchange
    exchange.cpp
)

target_link_libraries(exchange PRIVATE
    alex_library
)

# Set the target properties, if needed
target_compile_options(exchange PRIVATE 
    -Wall 
    -Wextra 
    -O2 
)

