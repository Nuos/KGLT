#This file is for other projects to use, so that they can locate KGLT
#using cmake

FIND_PATH( KGLT_INCLUDE_DIR NAMES kglt/kglt.h PATHS /usr/include /usr/local/include $ENV{INCLUDE} )
FIND_LIBRARY( KGLT_LIBRARY NAMES kglt PATHS /usr/lib /usr/local/lib )

IF(KGLT_INCLUDE_DIR)
	MESSAGE(STATUS "Found KGLT include dir: ${KGLT_INCLUDE_DIR}")
	SET(KGLT_INCLUDE_DIRS 
		${KGLT_INCLUDE_DIR} 
		${KGLT_INCLUDE_DIR}/kglt/deps
		${KGLT_INCLUDE_DIR}/kglt/deps/kazsignal
	)
ELSE(KGLT_INCLUDE_DIR)
	MESSAGE(STATUS "Could NOT find KGLT headers.")
ENDIF(KGLT_INCLUDE_DIR)

IF(KGLT_LIBRARY)
	MESSAGE(STATUS "Found KGLT library: ${KGLT_LIBRARY}")
ELSE(KGLT_LIBRARY)
	MESSAGE(STATUS "Could NOT find KGLT library.")
ENDIF(KGLT_LIBRARY)

IF(KGLT_INCLUDE_DIR AND KGLT_LIBRARY)
	SET(KGLT_FOUND "YES")
	SET(KGLT_LIBRARIES ${KGLT_LIBRARY})
ELSE(KGLT_INCLUDE_DIR AND KGLT_LIBRARY)
	SET(KGLT_FOUND "NO")
	IF(KGLT_FIND_REQUIRED)
		MESSAGE(FATAL_ERROR "Could not find KGLT. Please install KGLT (http://github.com/kazade/kglt)")
	ENDIF(KGLT_FIND_REQUIRED)
ENDIF(KGLT_INCLUDE_DIR AND KGLT_LIBRARY)