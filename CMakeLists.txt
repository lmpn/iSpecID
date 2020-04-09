cmake_minimum_required(VERSION 3.10)
project(iSpecID LANGUAGES CXX VERSION 1.0)
set(CMAKE_INCLUDE_CURRENT_DIR ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "-Wall -Wabi -g -fno-omit-frame-pointer -O2 -DNDEBUG -fno-inline-functions  -fno-optimize-sibling-calls")



OPTION(BIN "Create binary" ON)
OPTION(BENCH "Create benchmark binary" ON)



include_directories(
    ${PROJECT_SOURCE_DIR}/iSpecIDCore/include
    ${PROJECT_SOURCE_DIR}/iSpecIDCore/src
    ${PROJECT_SOURCE_DIR}/third_party/boost/include
    #${PROJECT_SOURCE_DIR}/third_party/curl/include
)
#link_directories(${PROJECT_SOURCE_DIR}/third_party/curl/lib)
link_directories(${PROJECT_SOURCE_DIR}/third_party/boost/lib)

set(CURL_LIBRARY "-lcurl") 
find_package(CURL REQUIRED)
include_directories(${CURL_INCLUDE_DIR})
file(GLOB core
    "${PROJECT_SOURCE_DIR}/iSpecIDCore/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/iSpecIDCore/include/*.h"
    )

if(BIN)
    if(COMMAND cmake_policy)
        cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)

    add_executable( isid "${PROJECT_SOURCE_DIR}/main.cpp" ${core}  )
    target_link_libraries(isid   ${CURL_LIBRARIES})
    target_link_libraries(isid  -lpthread)
endif()
IF(BENCH)
    if(COMMAND cmake_policy)
        cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)

    ADD_SUBDIRECTORY(third_party/benchmark)
    
    ADD_EXECUTABLE(isid_bench "${PROJECT_SOURCE_DIR}/iSpecIDCore/bench/annotate.cxx" ${core}  )
    TARGET_LINK_LIBRARIES(isid_bench  benchmark::benchmark)
    TARGET_LINK_LIBRARIES(isid_bench   ${CURL_LIBRARIES})
    TARGET_LINK_LIBRARIES(isid_bench  -lpthread)

    ADD_EXECUTABLE(isid_group "${PROJECT_SOURCE_DIR}/iSpecIDCore/bench/group.cxx" ${core}  )
    TARGET_LINK_LIBRARIES(isid_group  benchmark::benchmark)
    TARGET_LINK_LIBRARIES(isid_group   ${CURL_LIBRARIES})
    TARGET_LINK_LIBRARIES(isid_group  -lpthread)


    ADD_EXECUTABLE(isid_split "${PROJECT_SOURCE_DIR}/iSpecIDCore/bench/string_split.cxx")
    TARGET_LINK_LIBRARIES(isid_split  benchmark::benchmark)
ENDIF()
UNSET(BENCH CACHE)

unset(BIN CACHE)
