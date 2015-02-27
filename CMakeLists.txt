cmake_minimum_required (VERSION 2.8.7)

option(test "Build all tests." ON)
option(docs "Build all docs." ON)
option(coverage "Build with coverage support." OFF)

#cmake_policy(SET CMP0042 NEW)

project (dbabstract)
set(GTEST_ROOT "${CMAKE_SOURCE_DIR}/gtest-1.7.0")
set(CMAKE_CXX_FLAGS "-g -Wall -I${dbabstract_SOURCE_DIR}")
set(ARCHIVE_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/libs")
SET(CMAKE_MODULE_PATH "${CMAKE_MODULE_PATH};${CMAKE_SOURCE_DIR}/cmake-modules")

if (tests)
add_subdirectory(gtest-1.7.0)

include(CodeCoverage)
if (coverage)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -g -O0 -Wall -W -Wshadow -Wunused-variable -Wunused-parameter -Wunused-function -Wunused -Wno-system-headers -Wno-deprecated -Woverloaded-virtual -Wwrite-strings -fprofile-arcs -ftest-coverage")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -g -O0 -Wall -W -fprofile-arcs -ftest-coverage")
    set(CMAkE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} -fprofile-arcs -ftest-coverage")
    SETUP_TARGET_FOR_COVERAGE(coverage "${CMAKE_SOURCE_DIR}/test/tests" static coverage)
endif()
endif()

add_subdirectory(dbabstract)
if (docs)
add_subdirectory(doc)
endif()
if (test)
add_subdirectory(test)
endif()
