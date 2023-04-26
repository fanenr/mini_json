cmake_minimum_required(VERSION 3.15)
project(json_parser)


aux_source_directory(. main_src)
# add_executable(main ${main_src})
add_executable(main main.cc)


find_package(Catch2 REQUIRED)
target_link_libraries(main PRIVATE Catch2::Catch2WithMain)