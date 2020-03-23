cmake_minimum_required(VERSION 3.16)

project(iSpecID LANGUAGES CXX)

set(CMAKE_INCLUDE_CURRENT_DIR ON)

set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTORCC ON)
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

OPTION(BIN "BINARY" OFF)
OPTION(APP "APP" ON)
set(CMAKE_PREFIX_PATH "/Users/lmpn/Qt/5.14.1/clang_64/lib/cmake" CACHE PATH "")
find_package(Qt5 COMPONENTS Widgets REQUIRED)


FIND_PACKAGE(Boost REQUIRED)
FIND_PACKAGE(CURL REQUIRED)
SET(CURL_LIBRARY "-lcurl")
include_directories(
    ${PROJECT_SOURCE_DIR}/iSpecIDCore/include
    ${Boost_INCLUDE_DIR}
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
    target_link_libraries(isid PRIVATE ${CURL_LIBRARIES})
    target_link_libraries(isid PRIVATE -lpthread)
endif()
if(APP)

    file(GLOB gui
        "${PROJECT_SOURCE_DIR}/iSpecIDApp/*.cpp"
        "${PROJECT_SOURCE_DIR}/iSpecIDApp/*.h"
        "${PROJECT_SOURCE_DIR}/iSpecIDApp/*.ui"
        )
    if(COMMAND cmake_policy)
        cmake_policy(SET CMP0003 NEW)
    endif(COMMAND cmake_policy)

    add_executable( iSpecID "${PROJECT_SOURCE_DIR}/app.cpp" ${core} ${gui}  )
    target_link_libraries(iSpecID PRIVATE ${CURL_LIBRARIES})
    target_link_libraries(iSpecID PRIVATE -lpthread)
    target_link_libraries(iSpecID PRIVATE Qt5::Widgets)
endif()

unset(APP CACHE)
unset(BIN CACHE)
