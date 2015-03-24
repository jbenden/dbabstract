# - Find mysqlclient
# Find the native MySQL includes and library
#
#  MYSQL_INCLUDE_DIR - where to find mysql.h, etc.
#  MYSQL_LIBRARIES   - List of libraries when using MySQL.
#  MYSQL_FOUND       - True if MySQL found.

IF (MYSQL_INCLUDE_DIR)
  # Already in cache, be silent
  SET(MYSQL_FIND_QUIETLY TRUE)
ENDIF (MYSQL_INCLUDE_DIR)

FIND_PATH(MYSQL_INCLUDE_DIR mysql.h
  /usr/local/include/mysql
  /usr/include/mysql
)

SET(MYSQL_NAMES mysqlclient mysqlclient_r)
FIND_LIBRARY(MYSQL_LIBRARY
  NAMES ${MYSQL_NAMES}
  PATHS /usr/lib /usr/local/lib /usr/lib64/mysql /usr/lib64
)

IF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    SET(MYSQL_FOUND TRUE CACHE INTERNAL "Mysql Found")
  SET( MYSQL_LIBRARIES ${MYSQL_LIBRARY} )
ELSE (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)
    SET(MYSQL_FOUND FALSE CACHE INTERNAL "Mysql Found")
  SET( MYSQL_LIBRARIES )
ENDIF (MYSQL_INCLUDE_DIR AND MYSQL_LIBRARY)

IF (MYSQL_FOUND)
  IF (NOT MYSQL_FIND_QUIETLY)
    MESSAGE(STATUS "Found MySQL: ${MYSQL_LIBRARY}")
  ENDIF (NOT MYSQL_FIND_QUIETLY)
ELSE (MYSQL_FOUND)
  IF (MYSQL_FIND_REQUIRED)
    MESSAGE(STATUS "Looked for MySQL libraries named ${MYSQL_NAMES}.")
    MESSAGE(FATAL_ERROR "Could NOT find MySQL library")
  ENDIF (MYSQL_FIND_REQUIRED)
ENDIF (MYSQL_FOUND)

MARK_AS_ADVANCED(
  MYSQL_LIBRARY
  MYSQL_INCLUDE_DIR
  )

if (MYSQL_FOUND)
    add_library(mysql_dba MODULE mysql_db.cpp)
    add_library(mysql_dba_static STATIC mysql_db.cpp)
    set_target_properties(mysql_dba_static PROPERTIES OUTPUT_NAME mysql_dba)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${MYSQL_INCLUDE_DIR}")
    target_link_libraries(mysql_dba ${MYSQL_LIBRARY})
    target_link_libraries(mysql_dba_static ${MYSQL_LIBRARY})
    set_target_properties(mysql_dba_static PROPERTIES COMPILE_FLAGS -DSTATIC)
    set_target_properties(mysql_dba_static PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${ARCHIVE_OUTPUT_DIRECTORY})
    install(TARGETS mysql_dba
        LIBRARY DESTINATION lib)
    install(TARGETS mysql_dba_static
        ARCHIVE DESTINATION lib)
endif()

