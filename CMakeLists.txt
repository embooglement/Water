cmake_minimum_required (VERSION 3.1)
project (Water)

set (CMAKE_CXX_FLAGS "--std=gnu++11 ${CMAKE_C_FLAGS}")
file (GLOB SOURCE_FILES "source/*.cpp")

include_directories ("source")
add_executable (water ${SOURCE_FILES})