CMAKE_MINIMUM_REQUIRED (VERSION 3.10)
PROJECT(iSpecIDCore VERSION 1.0)

# specify the C++ standard
SET(CMAKE_CXX_STANDARD 17)
SET(CMAKE_CXX_STANDARD_REQUIRED True)
SET(CMAKE_CXX_FLAGS_RELEASE  "${CMAKE_CXX_FLAGS_RELEASE} -fno-omit-frame-pointer -Wall")

OPTION(BENCH "Do benchmark" ON)


FIND_PACKAGE(Boost REQUIRED)
FIND_PACKAGE(CURL REQUIRED) 
SET(CURL_LIBRARY "-lcurl")
include_directories(
        ${PROJECT_SOURCE_DIR}/include
        ${PROJECT_SOURCE_DIR}/src
        ${Boost_INCLUDE_DIR}
)
file(GLOB all_SRCS
        "${PROJECT_SOURCE_DIR}/src/*.cxx"
)

ADD_EXECUTABLE(isid ${all_SRCS} main.cxx )
TARGET_INCLUDE_DIRECTORIES(isid PUBLIC 
        "${PROJECT_BINARY_DIR}"
        "${PROJECT_SOURCE_DIR}/include"
        "${CURL_INCLUDE_DIR}"
        "${Boost_INCLUDE_DIR}"
)
TARGET_LINK_LIBRARIES(isid ${CURL_LIBRARIES} -lpthread)

IF(BENCH)
    ADD_SUBDIRECTORY(third_party/benchmark)
    ADD_SUBDIRECTORY(third_party/csv-parser)
    ADD_SUBDIRECTORY(modules/tsvreader)
    ADD_EXECUTABLE(benchRead ${all_SRCS} bench/read.cxx )
    ADD_EXECUTABLE(benchAnnotate ${all_SRCS} bench/annotate.cxx )

    TARGET_LINK_LIBRARIES(benchRead  benchmark::benchmark)
    TARGET_LINK_LIBRARIES(benchRead ${CURL_LIBRARIES})
    TARGET_LINK_LIBRARIES(benchRead -lpthread)
    TARGET_LINK_LIBRARIES(benchRead tsvreader)
    TARGET_LINK_LIBRARIES(benchAnnotate  benchmark::benchmark)
    TARGET_LINK_LIBRARIES(benchAnnotate ${CURL_LIBRARIES})
    TARGET_LINK_LIBRARIES(benchAnnotate -lpthread)
    
ENDIF()
UNSET(BENCH CACHE) 