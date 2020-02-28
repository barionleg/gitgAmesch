cmake_minimum_required (VERSION 3.10)
project (spherical_intersection VERSION 1.0.0 DESCRIPTION "mesh sphere intersection library")

option(BUILD_SPHERICAL_INTERSECTION_STANDALONE "Build standalone tool for spherical intersection" OFF)

add_library(spherical_intersection STATIC
									src/mesh.cpp	include/spherical_intersection/mesh.h
									src/graph.cpp	include/spherical_intersection/graph.h
									src/graph_geometry.cpp	include/spherical_intersection/graph_geometry.h
									src/graph_visualization.cpp	include/spherical_intersection/graph_visualization.h
											include/spherical_intersection/math3d/math3d.h
									src/algorithm/sphere_volume_msii.cpp	include/spherical_intersection/algorithm/sphere_volume_msii.h
									src/algorithm/sphere_intersections_msii.cpp	include/spherical_intersection/algorithm/sphere_intersections_msii.h
									src/algorithm/sphere_surface_msii.cpp		include/spherical_intersection/algorithm/sphere_surface_msii.h
									src/algorithm/component_count.cpp			include/spherical_intersection/algorithm/component_count.h
)

target_compile_features(spherical_intersection PRIVATE cxx_std_17)

target_include_directories(spherical_intersection PUBLIC
								$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
								$<INSTALL_INTERFACE:include>
							PRIVATE
								${CMAKE_CURRENT_SOURCE_DIR}/src
								${CMAKE_CURRENT_SOURCE_DIR}/include/spherical_intersection)

if(BUILD_SPHERICAL_INTERSECTION_STANDALONE)
	add_subdirectory(standalone)
endif()
