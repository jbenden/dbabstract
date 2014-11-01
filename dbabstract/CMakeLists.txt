add_subdirectory(mysql)
add_subdirectory(sqlite3)
add_subdirectory(pq)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${CMAKE_SOURCE_DIR}/dbabstract")

install(FILES db.h DESTINATION include)

