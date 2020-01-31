cmake_minimum_required(VERSION 3.10)

set(SOURCES_CORE mesh/geodentry.cpp
	mesh/bitflagarray.cpp
	mesh/primitive.cpp
	mesh/vertex.cpp
	mesh/vertexofface.cpp
	mesh/face.cpp
	mesh/sphere.cpp
	mesh/rectbox.cpp
	mesh/line.cpp
	mesh/userinteraction.cpp
	mesh/showprogress.cpp
	mesh/printbuildinfo.cpp
	mesh/polyline.cpp
	mesh/polyedge.cpp
	mesh/plane.cpp
	mesh/triangularprism.cpp
	mesh/cube.cpp
	mesh/triangle/triangle.c
	mesh/triangle/triangleTriangleIntersection.cpp
	mesh/edgegeodesic.cpp
	mesh/marchingfront.cpp
	mesh/meshio.cpp
	mesh/meshseedext.cpp
	mesh/mesh_params.cpp
	mesh/vector3d.cpp
	mesh/matrix4d.cpp
	mesh/voxelfilter25d.cpp
	mesh/meshinfodata.cpp
	mesh/mesh.cpp
	mesh/ellipsedisc.cpp
	mesh/MeshIO/MeshReader.cpp
	mesh/MeshIO/MeshWriter.cpp
	mesh/MeshIO/ModelMetaData.cpp
	mesh/MeshIO/ObjReader.cpp
	mesh/MeshIO/ObjWriter.cpp
	mesh/MeshIO/PlyReader.cpp
	mesh/MeshIO/PlyWriter.cpp
	mesh/MeshIO/RegularGridTxtReader.cpp
	mesh/MeshIO/TxtReader.cpp
	mesh/MeshIO/TxtWriter.cpp
	mesh/MeshIO/VRMLWriter.cpp
	mesh/MeshIO/MtlParser.cpp
	logging/Logging.cpp
	logging/Logger.cpp
	normalSphere/IcoSphereTree.cpp
	)

set(HEADERS_CORE
	mesh/MeshIO/MeshReader.h
	mesh/MeshIO/MeshWriter.h
	mesh/MeshIO/ObjReader.h
	mesh/MeshIO/ObjWriter.h
	mesh/MeshIO/PlyReader.h
	mesh/MeshIO/PlyWriter.h
	mesh/MeshIO/RegularGridTxtReader.h
	mesh/MeshIO/TxtReader.h
	mesh/MeshIO/TxtWriter.h
	mesh/MeshIO/VRMLWriter.h
	mesh/MeshIO/PlyEnums.h
	mesh/MeshIO/MtlParser.h
	mesh/triangle/triangle.h
	mesh/triangle/triangleTriangleIntersection.h
	)

set(PUBLIC_HEADERS_CORE ${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/printbuildinfo.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/logging/Logging.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/logging/Logger.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/mesh.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/geodentry.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/bitflagarray.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/primitive.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/vertex.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/vertexofface.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/face.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/sphere.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/rectbox.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/line.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/userinteraction.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/showprogress.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/polyline.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/polyedge.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/plane.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/triangularprism.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/cube.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/edgegeodesic.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/marchingfront.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/meshio.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/meshseedext.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/mesh_params.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/vector3d.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/matrix4d.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/voxelfilter25d.h
						${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/meshinfodata.h
						)

add_library(gigameshCore STATIC ${SOURCES_CORE} ${HEADERS_CORE} ${PUBLIC_HEADERS_CORE})

target_compile_features(gigameshCore PUBLIC cxx_std_17)
target_compile_options(gigameshCore PUBLIC -DTHREADS -DNUM_THREADS=11 -DCOMP_USER=\"${COMP_USER}\" -DCOMP_DATE=\"${COMP_DATE}\" -DCOMP_GITHEAD=\"${COMP_GITHEAD}\" -DVERSION_PACKAGE=\"${VERSION_PACKAGE}\" -DCOMP_EDIT=\"${COMP_EDIT}\" -DLIBSPHERICAL_INTERSECTION -DLIBPSALM -DALGLIB)

if(UNIX)
		target_link_libraries(gigameshCore PUBLIC stdc++fs pthread)
elseif(MSVC)
	target_compile_options(gigameshCore PUBLIC /permissive- -DNO_TIMER -DNOMINMAX)
	target_link_libraries(gigameshCore PUBLIC Advapi32 Ws2_32)
elseif(MINGW)
	target_link_libraries(gigameshCore PUBLIC ws2_32)
endif()

target_link_libraries(gigameshCore PRIVATE alg psalm spherical_intersection)

target_include_directories(gigameshCore PUBLIC
							$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
							$<INSTALL_INTERFACE:include>
							PRIVATE ${CMAKE_CURRENT_SOURCE_DIR})

#set_target_properties(gigameshCore PROPERTIES PUBLIC_HEADER ${PUBLIC_HEADERS_CORE}
#											  PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/ellipsedisc.h
#											  PUBLIC_HEADER	${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/MeshIO/ModelMetaData.h
#											  PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/mesh/gmcommon.h
#											  PUBLIC_HEADER ${CMAKE_CURRENT_SOURCE_DIR}/include/GigaMesh/normalSphere/IcoSphereTree.h
#											  )
