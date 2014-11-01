add_subdirectory(mysql)
add_subdirectory(sqlite3)
add_subdirectory(pq)

include_directories(${CMAKE_CURRENT_SOURCE_DIR})

install(FILES db.h DESTINATION include)

