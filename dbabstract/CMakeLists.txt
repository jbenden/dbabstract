add_subdirectory(mysql)
add_subdirectory(sqlite3)
add_subdirectory(pq)

set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I.. -I../dbabstract")

install(FILES db.h DESTINATION include)

