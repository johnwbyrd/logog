cmake_minimum_required (VERSION 2.6)
enable_testing()
project (logog)
set( RUNTIME_OUTPUT_DIRECTORY bin/ )
set( ARCHIVE_OUTPUT_DIRECTORY bin/ )
set( LIBRARY_OUTPUT_DIRECTORY bin/ )
set( CMAKE_BINARY_DIR build/ )
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
add_executable( test test/test.cpp )
target_link_libraries( test logog )
add_test( NAME test-harness COMMAND test )


