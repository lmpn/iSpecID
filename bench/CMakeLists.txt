cmake_minimum_required(VERSION 3.10)
project(benchAnnotate VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_executable(benchAnnotate annotate.cxx)
target_link_libraries(benchAnnotate annotatev1)
target_link_libraries(benchAnnotate annotatev2)
target_link_libraries(benchAnnotate annotatev3)
target_link_libraries(benchAnnotate annotatev4)
target_link_libraries(benchAnnotate annotatev5)
target_link_libraries(benchAnnotate annotatev6)
target_link_libraries(benchAnnotate benchmark::benchmark)