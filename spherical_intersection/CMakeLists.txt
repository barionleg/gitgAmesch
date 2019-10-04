cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 17)
project (spherical_intersection VERSION 1.0.0 DESCRIPTION "mesh sphere intersection library")
add_library(spherical_intersection
src/mesh.cpp
src/graph.cpp
src/graph.cpp
src/graph_geometry.cpp
src/graph_visualization.cpp
src/mesh.cpp
src/algorithm/sphere_volume_msii.cpp
src/algorithm/sphere_surface_msii.cpp
src/algorithm/component_count.cpp)
target_include_directories(spherical_intersection PRIVATE include)
set(DEBUG_OPTIONS "-g;-Wall;-Wextra;-pedantic")
set(RELEASE_OPTIONS "-O3")
target_compile_options(spherical_intersection PRIVATE "$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>")
target_compile_options(spherical_intersection PRIVATE "$<$<CONFIG:Release>:${RELEASE_OPTIONS}>")