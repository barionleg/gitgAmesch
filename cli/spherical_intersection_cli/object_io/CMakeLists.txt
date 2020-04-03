cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 17)
project (Object_IO VERSION 1.0.0 DESCRIPTION "A simple mesh input output library")
add_library(object_io src/obj.cpp src/ply.cpp)
target_include_directories(object_io PRIVATE include)

target_compile_options(object_io PRIVATE "$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>")
target_compile_options(object_io PRIVATE "$<$<CONFIG:Release>:${RELEASE_OPTIONS}>")
