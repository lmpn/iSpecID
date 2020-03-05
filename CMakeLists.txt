cmake_minimum_required(VERSION 3.10)
project(eCSI VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)

option(DEBUGP "Print intermediate results" OFF)
if(DEBUGP)
    add_definitions(-DDEBUG)
endif(DEBUGP)
unset(DEBUGP CACHE) 


add_subdirectory(third_party/csv-parser)
add_subdirectory(modules/tsvreader)
add_subdirectory(modules/miner)
add_subdirectory(modules/utils)
add_subdirectory(modules/annotatev1)
add_subdirectory(modules/annotatev2)
add_subdirectory(modules/annotatev3)
add_subdirectory(modules/annotatev4)
option(BENCH "Do benchmark" OFF)
if(BENCH)
    add_subdirectory(third_party/benchmark)
    add_subdirectory(bench)
endif()
unset(BENCH CACHE) 
add_executable(ecsi main.cxx)
target_include_directories(ecsi PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )
target_link_libraries(ecsi tsvreader)
target_link_libraries(ecsi miner)
target_link_libraries(ecsi utils)
target_link_libraries(ecsi annotatev1)
target_link_libraries(ecsi annotatev2)
target_link_libraries(ecsi annotatev3)
target_link_libraries(ecsi annotatev4)