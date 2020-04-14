cmake_minimum_required(VERSION 3.10)
project(iSpecID LANGUAGES CXX VERSION 1.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -Wabi -g -fno-omit-frame-pointer -O2 -DNDEBUG -fno-inline-functions -fno-optimize-sibling-calls")
set(CMAKE_PREFIX_PATH "/usr/local/Cellar/curlpp/0.8.1;${PROJECT_SOURCE_DIR}/third_party/curl;${PROJECT_SOURCE_DIR}/third_party/boost")

option(BIN "Create binary" ON)
option(BENCH "Create benchmark binary" OFF)


find_package(CURL REQUIRED)
find_package(Boost REQUIRED)
find_package(LibXml2 REQUIRED)
if(LibXml2_FOUND)
    message(STATUS "Using LibXml2 include dir(s): ${LIBXML2_INCLUDE_DIRS}")
    message(STATUS "Using LibXml2 lib(s): ${LIBXML2_LIBRARIES}")
else()
    message(FATAL_ERROR "Could not find CURL")
endif()
if(CURL_FOUND)
    message(STATUS "Found CURL version: ${CURL_VERSION_STRING}")
    message(STATUS "Using CURL include dir(s): ${CURL_INCLUDE_DIRS}")
    message(STATUS "Using CURL lib(s): ${CURL_LIBRARIES}")
else()
    message(FATAL_ERROR "Could not find CURL")
endif()
if(Boost_FOUND)
    message(STATUS "Found Boost version: ${Boost_VERSION}")
    message(STATUS "Using Boost include dir(s): ${Boost_INCLUDE_DIRS}")
else()
    message(FATAL_ERROR "Could not find Boost")
endif()



include_directories(
    ${PROJECT_SOURCE_DIR}/iSpecIDCore/include
    ${PROJECT_SOURCE_DIR}/iSpecIDCore/src
    ${Boost_INCLUDE_DIRS}
    ${CURL_INCLUDE_DIRS}
    ${LIBXML2_INCLUDE_DIRS}
)
file(GLOB core
    "${PROJECT_SOURCE_DIR}/iSpecIDCore/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/iSpecIDCore/include/*.h"
    )

if(BIN)
    if(COMMAND cmake_policy)
        cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)

    add_executable( isid "${PROJECT_SOURCE_DIR}/main.cpp" ${core}  )
    target_link_libraries(isid ${CURL_LIBRARIES})
    target_link_libraries(isid ${LIBXML2_LIBRARIES})
    target_link_libraries(isid -lpthread)
endif()
if(BENCH)
    if(COMMAND cmake_policy)
        cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)

    add_subdirectory(third_party/benchmark)

    add_executable(isid_bench "${PROJECT_SOURCE_DIR}/iSpecIDCore/bench/annotate.cxx" ${core}  )
    target_link_libraries(isid_bench  benchmark::benchmark)
    target_link_libraries(isid_bench ${CURL_LIBRARIES})
    target_link_libraries(isid_bench  -lpthread)

    add_executable(isid_group "${PROJECT_SOURCE_DIR}/iSpecIDCore/bench/group.cxx" ${core}  )
    target_link_libraries(isid_group  benchmark::benchmark)
    target_link_libraries(isid_group  ${CURL_LIBRARIES})
    target_link_libraries(isid_group  -lpthread)


    add_executable(isid_split "${PROJECT_SOURCE_DIR}/iSpecIDCore/bench/string_split.cxx")
    target_link_libraries(isid_split  benchmark::benchmark)
endif()
UNSET(BENCH CACHE)

unset(BIN CACHE)