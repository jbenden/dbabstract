add_subdirectory(mysql)
add_subdirectory(sqlite3)
add_subdirectory(pq)

include_directories(BEFORE ${CMAKE_SOURCE_DIR}/dbabstract)

install(FILES db.h DESTINATION include)

