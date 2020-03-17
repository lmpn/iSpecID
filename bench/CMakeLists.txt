cmake_minimum_required(VERSION 3.10)
project(benchAnnotate VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

add_executable(benchRead read.cxx "${PROJECT_SOURCE_DIR}/src/utils.cxx")
target_link_libraries(benchRead tsvreader)
target_link_libraries(benchRead benchmark::benchmark)