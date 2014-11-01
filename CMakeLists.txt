cmake_minimum_required (VERSION 2.8.7)

option(test "Build all tests." ON)

#cmake_policy(SET CMP0042 NEW)

project (dbabstract)
set(CMAKE_CXX_FLAGS "-g -Wall -I${CMAKE_SOURCE_DIR}/dbabstract")

add_subdirectory(dbabstract)
add_subdirectory(doc)
add_subdirectory(test)
