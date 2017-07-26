# cmakelists.txt for logog
cmake_minimum_required (VERSION 2.8.4)

# Compile-time constants
set( LOGOG_UNICODE FALSE CACHE BOOL "Enable Unicode support for output strings.")
set( LOGOG_USE_PREFIX FALSE CACHE BOOL "Apply a LOGOG_ prefix to all logging macros; helps avoid conflicts with existing code" )
set( LOGOG_LEAK_DETECTION FALSE CACHE BOOL "Enable detection of internal leaks in the library")
set( LOGOG_LEAK_DETECTION_MICROSOFT CACHE BOOL "Enable detection of internal leaks using Microsoft's crt mechanism" )
set( LOGOG_UNIT_TESTING FALSE CACHE BOOL "Enable logog's internal unit testing framework.")
set( LOGOG_USE_COTIRE FALSE CACHE BOOL "Use cotire to speed up builds.")

if( LOGOG_USE_COTIRE )
	set (CMAKE_MODULE_PATH "${CMAKE_SOURCE_DIR}/CMake")	
	include(cotire OPTIONAL)
endif()

if ( LOGOG_LEAK_DETECTION AND LOGOG_LEAK_DETECTION_MICROSOFT )
	message( FATAL_ERROR "Can't enable both LOGOG_LEAK_DETECTION and LOGOG_LEAK_DETECTION_MICROSOFT, choose one or the other")
endif() 

if( LOGOG_UNICODE )
	add_definitions( -DLOGOG_UNICODE )
endif()
if( LOGOG_USE_PREFIX )
	add_definitions( -DLOGOG_USE_PREFIX )
endif()
if( LOGOG_LEAK_DETECTION )
	add_definitions( -DLOGOG_LEAK_DETECTION )
endif()
if( LOGOG_LEAK_DETECTION_MICROSOFT )
	add_definitions( -DLOGOG_LEAK_DETECTION_MICROSOFT )
endif()
if( LOGOG_UNIT_TESTING )
	add_definitions( -DLOGOG_UNIT_TESTING )
endif()

enable_testing()
project (logog)
include(CTest)

# logog needs thread support on linux
find_package( Threads )

set( RUNTIME_OUTPUT_DIRECTORY bin/ )
set( ARCHIVE_OUTPUT_DIRECTORY bin/ )
set( LIBRARY_OUTPUT_DIRECTORY bin/ )
set( CMAKE_LEGACY_CYGWIN_WIN32 0 )

if(MSVC)
  # Force to always compile with W4; all warnings are errors
  if(CMAKE_CXX_FLAGS MATCHES "/W[0-4]")
    string(REGEX REPLACE "/W[0-4]" "/W4 /WX" CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS}")
  else()
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} /W4 /WX")
  endif()
elseif(CMAKE_COMPILER_IS_GNUCC OR CMAKE_COMPILER_IS_GNUCXX)
  # Warnings are errors; generate lots of warnings
  set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -Wno-variadic-macros -Wall -Werror -Wno-long-long")
endif()

include_directories( include )
add_library( logog
	src/api.cpp 
	src/checkpoint.cpp
	src/formatter.cpp
	src/lobject.cpp
	src/lstring.cpp
	src/message.cpp
	src/mutex.cpp
	src/node.cpp
	src/platform.cpp
	src/socket.cpp
	src/statics.cpp
	src/target.cpp
	src/timer.cpp
	src/topic.cpp
	src/unittest.cpp
)

set_target_properties(logog PROPERTIES DEBUG_POSTFIX "d")
add_executable( test-logog test/test.cpp )
target_link_libraries( test-logog logog ${CMAKE_THREAD_LIBS_INIT})
if( LOGOG_USE_COTIRE )
	if (COMMAND cotire)
		cotire( logog )
	endif()
endif()

add_test( NAME test-harness COMMAND test-logog )
install(TARGETS logog ARCHIVE DESTINATION ${CMAKE_INSTALL_PREFIX}/lib)
install(DIRECTORY include/ DESTINATION "${CMAKE_INSTALL_PREFIX}/include/logog"
          FILES_MATCHING PATTERN "*.hpp")

# Docs generation with Doxygen
find_package( Doxygen )
if( DOXYGEN_FOUND )
	add_custom_target (doc ${DOXYGEN_EXECUTABLE} ${CMAKE_SOURCE_DIR}/doxyfile
		WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
		COMMENT "Generating source code documentation with Doxygen." VERBATIM)
endif() # DOXYGEN_FOUND