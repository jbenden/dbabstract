add_subdirectory(mysql)
add_subdirectory(sqlite3)
add_subdirectory(pq)
add_subdirectory(odbc)

install(FILES db.h DESTINATION include/dbabstract)

