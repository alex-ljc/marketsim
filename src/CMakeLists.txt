# Generates the penguins executable
add_executable(marketsim
    main.cpp
)

target_link_libraries(marketsim PRIVATE
    alex_library
)

# Set the target properties, if needed
target_compile_options(marketsim PRIVATE 
    -Wall 
    -Wextra 
    -O2 
)

