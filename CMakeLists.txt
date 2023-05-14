cmake_minimum_required(VERSION 3.15)
project(mini_json)


# rebuild
add_custom_target(
    build
    COMMAND cmake ..
)

# run test
add_subdirectory(test)
add_custom_target(
    run_t
    COMMAND cmake --build . 
    COMMAND test
    DEPENDS test
)

# run benchmark
add_custom_target(
    run_b
    COMMAND cmake --build . 
    COMMAND bench
    DEPENDS bench
)

# run memory check
add_custom_target(
    run_m
    COMMAND cmake --build . 
    COMMAND valgrind --leak-check=full test/test
    DEPENDS test
)