# Find the StealthLink2.0 Libraries# This module defines# STEALTHLINK_INCLUDE_DIRS - STEALTHLINK header files# STEALTHLINK_STEALTHLINK_STATIC_LIBRARY  - STEALTHLINK static library# STEALTHLINK_STEALTHLINK_SHARED_LIBRARY  - STEALTHLINK shared library (only on Windows)# STEALTHLINK_STEALTHLINKD_STATIC_LIBRARY - STEALTHLINK static library with debug info (only on Windows)# STEALTHLINK_STEALTHLINKD_SHARED_LIBRARY - STEALTHLINK shared library with debug info (only on Windows)#SET( STEALTHLINK_PATH_HINTS  ../StealthLink-2.0.1  ../PLTools/StealthLink-2.0.1  ../../PLTools/StealthLink-2.0.1  ../trunk/PLTools/StealthLink-2.0.1  ${CMAKE_CURRENT_BINARY_DIR}/StealthLink-2.0.1  )IF (WIN32)  IF(NOT ${CMAKE_GENERATOR} MATCHES "Visual Studio 10" )    MESSAGE(FATAL_ERROR "error: StealthLink can only be built using Visual Studio 2010")  ENDIF(NOT ${CMAKE_GENERATOR} MATCHES "Visual Studio 10" )  IF (CMAKE_CL_64)    SET( PLATFORM_SUFFIX "/windows/x64/Release")    SET( PLATFORM_SUFFIXD "/windows/x64/Debug")  ELSE()    SET (PLATFORM_SUFFIX "/windows/Win32/Release")    SET (PLATFORM_SUFFIXD "/windows/Win32/Debug")  ENDIF()ELSE()  if(CMAKE_SIZEOF_VOID_P EQUAL 8)    SET (PLATFORM_SUFFIX "i686-linux-gnu3")  endif(CMAKE_SIZEOF_VOID_P EQUAL 8)ENDIF()FIND_PATH (STEALTHLINK_INCLUDE_DIRS  NAMES "StealthLink/Stealthlink.h"  PATHS ${STEALTHLINK_PATH_HINTS}  DOC "Include directory, i.e. parent directory of directory \"StealthLink\""  )FIND_LIBRARY (STEALTHLINK_STEALTHLINK_STATIC_LIBRARY  NAMES StealthLink  PATH_SUFFIXES ${PLATFORM_SUFFIX}  PATHS ${STEALTHLINK_PATH_HINTS}  )IF (WIN32)  FIND_FILE (STEALTHLINK_STEALTHLINK_SHARED_LIBRARY    NAMES StealthLink${CMAKE_SHARED_LIBRARY_SUFFIX}    PATH_SUFFIXES ${PLATFORM_SUFFIX}    PATHS ${STEALTHLINK_PATH_HINTS}    )  FIND_LIBRARY (STEALTHLINK_STEALTHLINKD_STATIC_LIBRARY    NAMES StealthLink    PATH_SUFFIXES ${PLATFORM_SUFFIXD}    PATHS ${STEALTHLINK_PATH_HINTS}    )  FIND_FILE (STEALTHLINK_STEALTHLINKD_SHARED_LIBRARY    NAMES StealthLink${CMAKE_SHARED_LIBRARY_SUFFIX}    PATH_SUFFIXES ${PLATFORM_SUFFIXD}    PATHS ${STEALTHLINK_PATH_HINTS}    )ENDIF()INCLUDE(FindPackageHandleStandardArgs)IF (WIN32)  FIND_PACKAGE_HANDLE_STANDARD_ARGS(STEALTHLINK DEFAULT_MSG    STEALTHLINK_INCLUDE_DIRS    STEALTHLINK_STEALTHLINK_STATIC_LIBRARY    STEALTHLINK_STEALTHLINK_SHARED_LIBRARY    STEALTHLINK_STEALTHLINKD_STATIC_LIBRARY    STEALTHLINK_STEALTHLINKD_SHARED_LIBRARY    )ELSE()  FIND_PACKAGE_HANDLE_STANDARD_ARGS(STEALTHLINK DEFAULT_MSG    STEALTHLINK_INCLUDE_DIRS    STEALTHLINK_STEALTHLINK_STATIC_LIBRARY    )ENDIF()