# - Find sqlite3
# Find the native SQLite3 includes and library
#
#  SQLITE_INCLUDE_DIR - where to find mysql.h, etc.
#  SQLITE_LIBRARIES   - List of libraries when using MySQL.
#  SQLITE_FOUND       - True if MySQL found.

IF (SQLITE_INCLUDE_DIR)
  # Already in cache, be silent
  SET(SQLITE_FIND_QUIETLY TRUE)
ENDIF (SQLITE_INCLUDE_DIR)

FIND_PATH(SQLITE_INCLUDE_DIR sqlite3.h
  /usr/local/include/sqlite3
  /usr/include/sqlite3
  /usr/include
)

SET(SQLITE_NAMES sqlite3)
FIND_LIBRARY(SQLITE_LIBRARY
    NAMES ${SQLITE_NAMES}
  PATHS /usr/lib /usr/local/lib
  PATH_SUFFIXES sqlite3
)

IF (SQLITE_INCLUDE_DIR AND SQLITE_LIBRARY)
    SET(SQLITE_FOUND TRUE PARENT_SCOPE)
    SET( SQLITE_LIBRARIES ${SQLITE_LIBRARY} )
ELSE (SQLITE_INCLUDE_DIR AND SQLITE_LIBRARY)
    SET(SQLITE_FOUND FALSE PARENT_SCOPE)
    SET( SQLITE_LIBRARIES )
ENDIF (SQLITE_INCLUDE_DIR AND SQLITE_LIBRARY)

IF (SQLITE_FOUND)
    IF (NOT SQLITE_FIND_QUIETLY)
        MESSAGE(STATUS "Found SQLite3: ${SQLITE_LIBRARY}")
    ENDIF (NOT SQLITE_FIND_QUIETLY)
ELSE (SQLITE_FOUND)
    IF (SQLITE_FIND_REQUIRED)
        MESSAGE(STATUS "Looked for Sqlite3 libraries named ${SQLITE_NAMES}.")
        MESSAGE(FATAL_ERROR "Could NOT find Sqlite3 library")
    ENDIF (SQLITE_FIND_REQUIRED)
ENDIF (SQLITE_FOUND)

MARK_AS_ADVANCED(
    SQLITE_LIBRARY
    SQLITE_INCLUDE_DIR
  )

if (SQLITE_FOUND)
    add_library(sqlite3_dba MODULE sqlite3_db.cpp)
    add_library(sqlite3_dba_static STATIC sqlite3_db.cpp)
    set_target_properties(sqlite3_dba_static PROPERTIES OUTPUT_NAME sqlite3_dba)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${SQLITE_INCLUDE_DIR}")
    target_link_libraries(sqlite3_dba ${SQLITE_LIBRARY})
    target_link_libraries(sqlite3_dba_static ${SQLITE_LIBRARY})
    install(TARGETS sqlite3_dba LIBRARY DESTINATION lib)
    install(TARGETS sqlite3_dba_static ARCHIVE DESTINATION lib)
endif()

