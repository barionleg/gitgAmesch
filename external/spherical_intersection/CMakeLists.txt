cmake_minimum_required (VERSION 3.1)
set (CMAKE_CXX_STANDARD 17)
project (spherical_intersection VERSION 1.0.0 DESCRIPTION "mesh sphere intersection library")
add_library(spherical_intersection STATIC
									src/mesh.cpp
									src/graph.cpp
									src/graph.cpp
									src/graph_geometry.cpp
									src/graph_visualization.cpp
									src/mesh.cpp
									src/algorithm/sphere_volume_msii.cpp
									src/algorithm/sphere_intersections_msii.cpp
									src/algorithm/sphere_surface_msii.cpp
									src/algorithm/component_count.cpp)
target_include_directories(spherical_intersection PRIVATE include/spherical_intersection)
#if (NOT CMAKE_BUILD_TYPE)
#	message(STATUS "Using default build type: Release")
#	set(CMAKE_BUILD_TYPE Release)
#endif()
#set(DEBUG_OPTIONS "-g;-Wall;-Wextra;-pedantic")
#set(RELEASE_OPTIONS "-O3")
#target_compile_options(spherical_intersection PRIVATE "$<$<CONFIG:Debug>:${DEBUG_OPTIONS}>")
#target_compile_options(spherical_intersection PRIVATE "$<$<CONFIG:Release>:${RELEASE_OPTIONS}>")

target_include_directories(spherical_intersection PUBLIC
							$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
							$<INSTALL_INTERFACE:include>
							PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

set_target_properties(spherical_intersection PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/graph.h
															${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/graph_geometry.h
															${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/graph_visualization.h
															${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/mesh.h
															${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/math3d/math3d.h
															${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/algorithm/component_count.h
															${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/algorithm/sphere_intersections_msii.h
															${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/algorithm/sphere_surface_msii.h
															${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection/algorithm/sphere_colume_msii.h
															)
