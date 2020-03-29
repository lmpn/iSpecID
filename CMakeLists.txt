cmake_minimum_required(VERSION 3.10)

project(iSpecID LANGUAGES CXX VERSION 1.0)



set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(CMAKE_CXX_FLAGS_PROFILLING "-Wall -Wabi -g -fno-omit-frame-pointer -O2 -DNDEBUG -fno-inline-functions -fno-inline-functions-called-once -fno-optimize-sibling-calls -pg" CACHE STRING
 "Flags used by the C++ compiler during Profilling builds."
  FORCE)
mark_as_advanced(
  CMAKE_CXX_FLAGS_PROFILLING
  CMAKE_C_FLAGS_PROFILLING
  CMAKE_EXE_LINKER_FLAGS_PROFILLING
  CMAKE_SHARED_LINKER_FLAGS_PROFILLING)


set(CMAKE_BUILD_TYPE "${CMAKE_BUILD_TYPE}" CACHE STRING
  "Choose the type of build, options are: None Debug Release RelWithDebInfo MinSizeRel Profilling."
FORCE)



OPTION(BIN "BINARY" ON)
OPTION(APP "APP" OFF)
OPTION(BENCH "Do benchmark" OFF)



FIND_PACKAGE(Boost REQUIRED)
include_directories(
    ${PROJECT_SOURCE_DIR}/third_party/curl/include
    ${PROJECT_SOURCE_DIR}/iSpecIDCore/include
    ${PROJECT_SOURCE_DIR}/iSpecIDCore/src
    ${Boost_INCLUDE_DIR}
)
link_directories(${PROJECT_SOURCE_DIR}/third_party/curl/lib)



file(GLOB core
    "${PROJECT_SOURCE_DIR}/iSpecIDCore/src/*.cpp"
    "${PROJECT_SOURCE_DIR}/iSpecIDCore/include/*.h"
    )

if(BIN)
    if(COMMAND cmake_policy)
        cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)

    add_executable( isid "${PROJECT_SOURCE_DIR}/main.cpp" ${core}  )
    target_link_libraries(isid  -lcurl)
    target_link_libraries(isid  -lpthread)
endif()
IF(BENCH)
    if(COMMAND cmake_policy)
        cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)

    ADD_SUBDIRECTORY(third_party/benchmark)
    ADD_EXECUTABLE(isid_bench "${PROJECT_SOURCE_DIR}/iSpecIDCore/bench/annotate.cxx" ${core}  )
    TARGET_LINK_LIBRARIES(isid_bench  benchmark::benchmark)
    TARGET_LINK_LIBRARIES(isid_bench  -lcurl)
    TARGET_LINK_LIBRARIES(isid_bench  -lpthread)
    ADD_EXECUTABLE(isid_split "${PROJECT_SOURCE_DIR}/iSpecIDCore/bench/string_split.cxx")
    TARGET_LINK_LIBRARIES(isid_split  benchmark::benchmark)
ENDIF()
UNSET(BENCH CACHE)
if(APP)
    set(CMAKE_AUTOUIC ON)
    set(CMAKE_AUTOMOC ON)
    set(CMAKE_AUTORCC ON)
    set(CMAKE_PREFIX_PATH "/Users/lmpn/Qt/5.14.1/clang_64/lib/cmake" CACHE PATH "")
    find_package(Qt5 COMPONENTS Widgets REQUIRED)

    file(GLOB gui
        "${PROJECT_SOURCE_DIR}/iSpecIDApp/*.cpp"
        "${PROJECT_SOURCE_DIR}/iSpecIDApp/*.h"
        "${PROJECT_SOURCE_DIR}/iSpecIDApp/*.ui"
        )
    if(COMMAND cmake_policy)
        cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)

    add_executable( iSpecID "${PROJECT_SOURCE_DIR}/app.cpp" ${core} ${gui}  )
    target_link_libraries(iSpecID PRIVATE -lcurl)
    target_link_libraries(iSpecID PRIVATE -lpthread)
    target_link_libraries(iSpecID PRIVATE Qt5::Widgets)
endif()

unset(APP CACHE)
unset(BIN CACHE)
