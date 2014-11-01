# - Find pq
# Find the native PostgreSQL includes and library
#
#  PQ_INCLUDE_DIR - where to find mysql.h, etc.
#  PQ_LIBRARIES   - List of libraries when using PostgreSQL.
#  PQ_FOUND       - True if PostgreSQL found.

IF (PQ_INCLUDE_DIR)
  # Already in cache, be silent
  SET(PQ_FIND_QUIETLY TRUE)
ENDIF (PQ_INCLUDE_DIR)

FIND_PATH(PQ_INCLUDE_DIR pg_config.h
  /Applications/Postgres.app/Contents/Versions/9.3/include
  /usr/local/include
  /usr/include
)

SET(PQ_NAMES pq)
FIND_LIBRARY(PQ_LIBRARY
  NAMES ${PQ_NAMES}
  PATHS /usr/lib /usr/local/lib /Applications/Postgres.app/Contents/Versions/9.3/lib
)

IF (PQ_INCLUDE_DIR AND PQ_LIBRARY)
    SET(PQ_FOUND TRUE CACHE INTERNAL "Pq found.")
  SET( PQ_LIBRARIES ${PQ_LIBRARY} )
ELSE (PQ_INCLUDE_DIR AND PQ_LIBRARY)
    SET(PQ_FOUND FALSE CACHE INTERNAL "Pq found.")
  SET( PQ_LIBRARIES )
ENDIF (PQ_INCLUDE_DIR AND PQ_LIBRARY)

IF (PQ_FOUND)
  IF (NOT PQ_FIND_QUIETLY)
      MESSAGE(STATUS "Found PostgreSQL: ${PQ_LIBRARY}")
  ENDIF (NOT PQ_FIND_QUIETLY)
ELSE (PQ_FOUND)
  IF (PQ_FIND_REQUIRED)
      MESSAGE(STATUS "Looked for PostgreSQL libraries named ${PQ_NAMES}.")
    MESSAGE(FATAL_ERROR "Could NOT find PostgreSQL library")
  ENDIF (PQ_FIND_REQUIRED)
ENDIF (PQ_FOUND)

MARK_AS_ADVANCED(
  PQ_LIBRARY
  PQ_INCLUDE_DIR
  )

if (PQ_FOUND)
    add_library(pq_dba MODULE pq_db.cpp)
    add_library(pq_dba_static STATIC pq_db.cpp)
    set_target_properties(pq_dba_static PROPERTIES OUTPUT_NAME pq_dba)
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -I${PQ_INCLUDE_DIR}")
    target_link_libraries(pq_dba ${PQ_LIBRARY})
    target_link_libraries(pq_dba_static ${PQ_LIBRARY})
    set_target_properties(pq_dba_static PROPERTIES COMPILE_FLAGS -DSTATIC)
    set_target_properties(pq_dba_static PROPERTIES ARCHIVE_OUTPUT_DIRECTORY ${ARCHIVE_OUTPUT_DIRECTORY})
    install(TARGETS pq_dba LIBRARY DESTINATION lib)
    install(TARGETS pq_dba_static ARCHIVE DESTINATION lib)
endif()

