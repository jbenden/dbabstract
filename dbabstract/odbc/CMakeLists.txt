# - Find odbc library
#
#  ODBC_INCLUDE_DIR - where to find mysql.h, etc.
#  ODBC_LIBRARIES   - List of libraries when using MySQL.
#  ODBC_FOUND       - True if MySQL found.

IF (ODBC_INCLUDE_DIR)
  # Already in cache, be silent
  SET(ODBC_FIND_QUIETLY FALSE)
ENDIF (ODBC_INCLUDE_DIR)

FIND_PATH(ODBC_INCLUDE_DIR sql.h
  /usr/local/include
  /usr/include
)

SET(ODBC_NAMES odbc)
FIND_LIBRARY(ODBC_LIBRARY
    NAMES ${ODBC_NAMES}
  PATHS /usr/lib /usr/local/lib
)

IF (ODBC_INCLUDE_DIR AND ODBC_LIBRARY)
    SET(ODBC_FOUND TRUE CACHE INTERNAL "ODBC Found")
    SET( ODBC_LIBRARIES ${ODBC_LIBRARY} )
ELSE (ODBC_INCLUDE_DIR AND ODBC_LIBRARY)
    SET(ODBC_FOUND FALSE CACHE INTERNAL "ODBC Found")
    SET( ODBC_LIBRARIES )
ENDIF (ODBC_INCLUDE_DIR AND ODBC_LIBRARY)

IF (ODBC_FOUND)
    IF (NOT ODBC_FIND_QUIETLY)
        MESSAGE(STATUS "Found ODBC: ${ODBC_LIBRARY}")
    ENDIF (NOT ODBC_FIND_QUIETLY)
ELSE (ODBC_FOUND)
    IF (ODBC_FIND_REQUIRED)
        MESSAGE(STATUS "Looked for ODBC libraries named ${ODBC_NAMES}.")
        MESSAGE(FATAL_ERROR "Could NOT find ODBC library")
    ENDIF (ODBC_FIND_REQUIRED)
ENDIF (ODBC_FOUND)

MARK_AS_ADVANCED(
    ODBC_LIBRARY
    ODBC_INCLUDE_DIR
  )

if (ODBC_FOUND)
    add_library(odbc_dba MODULE odbc_db.cpp)
    add_library(odbc_dba_static STATIC odbc_db.cpp)
    set_target_properties(odbc_dba_static PROPERTIES OUTPUT_NAME odbc_dba)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${ODBC_INCLUDE_DIR}")
    target_link_libraries(odbc_dba ${ODBC_LIBRARY})
    target_link_libraries(odbc_dba_static ${ODBC_LIBRARY})
    set_target_properties(odbc_dba_static PROPERTIES COMPILE_FLAGS -DSTATIC)
    set_target_properties(odbc_dba_static PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${ARCHIVE_OUTPUT_DIRECTORY})
    install(TARGETS odbc_dba
        LIBRARY DESTINATION lib)
    install(TARGETS odbc_dba_static
        ARCHIVE DESTINATION lib)
endif()

