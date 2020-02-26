cmake_minimum_required(VERSION 3.10)
project(eCSI VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)
add_subdirectory(third_party/benchmark)
add_subdirectory(third_party/csvparser)



set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED) 
find_package(Boost REQUIRED)

add_executable(ecsi main.cxx)
include_directories(${CURL_INCLUDE_DIR})
include_directories(${boost_include_dir} )
include_directories(third_party/csvparser/include)
target_link_libraries(ecsi ${CURL_LIBRARIES})
target_link_libraries(ecsi benchmark::benchmark)
target_link_libraries(ecsi csv)
target_include_directories(ecsi PUBLIC
                           "${PROJECT_BINARY_DIR}"
                           )



