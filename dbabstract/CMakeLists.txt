add_subdirectory(mysql)
add_subdirectory(sqlite3)
add_subdirectory(pq)

set(MYSQL_FOUND ${MYSQL_FOUND} PARENT_SCOPE)
set(SQLITE_FOUND ${SQLITE_FOUND} PARENT_SCOPE)
set(PQ_FOUND ${PQ_FOUND} PARENT_SCOPE)

install(FILES db.h DESTINATION include)

