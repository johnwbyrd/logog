cmake_minimum_required (VERSION 2.6)
project (logog)
include_directories( include )
add_library( logog src/*.cpp )
add_executable( test test/*.cpp )
add_dependencies( test logog )


