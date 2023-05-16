project(mini_json_test)


add_executable(test test_node.cpp test_json.cpp)
add_executable(bench benchmark.cpp)
target_include_directories(test PRIVATE ../include)
target_include_directories(bench PRIVATE ../include)


find_package(Catch2 REQUIRED)
find_package(Boost  REQUIRED)
# need boost-optional headers
include_directories(${Boost_INCLUDE_DIRS})
target_link_libraries(test PRIVATE Catch2::Catch2WithMain)
target_link_libraries(bench PRIVATE Catch2::Catch2WithMain)