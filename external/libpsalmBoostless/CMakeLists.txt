cmake_minimum_required(VERSION 3.10)

project(libpsalm LANGUAGES CXX)



add_library(psalm STATIC
	src/libpsalm.cpp
	src/directed_edge.cpp
	src/edge.cpp
	src/face.cpp
	src/mesh.cpp
	src/v3ctor.cpp
	src/vertex.cpp
	src/FairingAlgorithms/FairingAlgorithm.cpp
	src/SegmentationAlgorithms/PlanarSegmentation.cpp
	src/SegmentationAlgorithms/SegmentationAlgorithm.cpp
	src/SubdivisionAlgorithms/BsplineSubdivisionAlgorithm.cpp
	src/SubdivisionAlgorithms/CatmullClark.cpp
	src/SubdivisionAlgorithms/DooSabin.cpp
	src/SubdivisionAlgorithms/Liepa.cpp
	src/SubdivisionAlgorithms/Loop.cpp
	src/SubdivisionAlgorithms/SubdivisionAlgorithm.cpp
	src/TriangulationAlgorithms/MinimumWeightTriangulation.cpp
	src/TriangulationAlgorithms/TriangulationAlgorithm.cpp
)

target_include_directories(psalm PUBLIC
							$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
							$<INSTALL_INTERFACE:include>
							PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_compile_features(psalm PRIVATE cxx_std_17)

set_target_properties(psalm PROPERTIES PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/libpsalm/libpsalm.h)

install(TARGETS psalm
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		PUBLIC_HEADER DESTINATION include/libpsalm)
