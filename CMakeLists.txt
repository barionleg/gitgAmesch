cmake_minimum_required(VERSION 3.10)

# CMAKEFile for building the command-line tools of gigamesh

# note for building the CLI for Ubuntu, if g++-8 is not the default compiler, you have two options:
# 1. Export CC and CXX before calling cmake:
# export CC=/usr/bin/gcc-8
# export CXX=/usr/bin/g++-8
#
# 2. set g++-8 as alternative permanently:
# sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 10
# sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 10

project(GigaMesh-CLT LANGUAGES CXX C)

string(TIMESTAMP COMP_DATE "%d.%b.%y")
execute_process(COMMAND git rev-parse HEAD
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE COMP_GITHEAD)
string(STRIP "${COMP_GITHEAD}" COMP_GITHEAD)

execute_process(COMMAND git log -1 --format=%ci
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE VERSION_PACKAGE)
set(COMP_EDIT IWR\ FreeWare)
string(STRIP "${VERSION_PACKAGE}" VERSION_PACKAGE)
string(SUBSTRING "${VERSION_PACKAGE}" 2 9 VERSION_PACKAGE)
string(REPLACE "-" "" VERSION_PACKAGE "${VERSION_PACKAGE}")

if(UNIX OR MINGW)
        execute_process(COMMAND whoami OUTPUT_VARIABLE WHOAMIVAR)
        execute_process(COMMAND hostname -s OUTPUT_VARIABLE HOSTVAR)

	string (STRIP "${WHOAMIVAR}" WHOAMIVAR)
	string (STRIP "${HOSTVAR}" HOSTVAR)
	string(CONCAT COMP_USER ${WHOAMIVAR} @ ${HOSTVAR})
elseif(MSVC)
    execute_process(COMMAND whoami OUTPUT_VARIABLE WHOAMIVAR)
    string(STRIP "${WHOAMIVAR}" WHOAMIVAR)
    string(FIND "${WHOAMIVAR}" "\\" SLASHPOS REVERSE)

    string(SUBSTRING "${WHOAMIVAR}" 0 ${SLASHPOS} HOST_VAR)
    math(EXPR SLASHPOS "${SLASHPOS} + 1")
    string(SUBSTRING "${WHOAMIVAR}" "${SLASHPOS}" -1 USER_VAR)
    string(CONCAT COMP_USER ${USER_VAR} @ ${HOST_VAR})
endif()

#--GENERAL LIBRARY STUFF SHARED BY ALL TOOLS--
add_subdirectory(external/libpsalmBoostless)

set(SOURCES_COMMON mesh/geodentry.cpp
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
	normalSphereSelection/IcoSphereTree.cpp
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
	logging/Logger.cpp)

set(HEADERS_COMMON mesh/geodentry.h
	mesh/bitflagarray.h
	mesh/primitive.h
	mesh/vertex.h
	mesh/vertexofface.h
	mesh/face.h
	mesh/sphere.h
	mesh/rectbox.h
	mesh/line.h
	mesh/userinteraction.h
	mesh/showprogress.h
	mesh/printbuildinfo.h
	mesh/polyline.h
	mesh/polyedge.h
	mesh/plane.h
	mesh/triangularprism.h
	mesh/cube.h
	mesh/triangle/triangle.h
	mesh/triangle/triangleTriangleIntersection.h
	mesh/edgegeodesic.h
	mesh/marchingfront.h
	mesh/meshio.h
	mesh/meshseedext.h
	mesh/mesh_params.h
	mesh/vector3d.h
	mesh/matrix4d.h
	mesh/voxelfilter25d.h
	mesh/meshinfodata.h
	mesh/mesh.h
	normalSphereSelection/IcoSphereTree.h
	mesh/MeshIO/MeshReader.h
	mesh/MeshIO/MeshWriter.h
	mesh/MeshIO/ModelMetaData.h
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
	logging/Logging.h
	logging/Logger.h)

if(MSVC)
    list(APPEND HEADERS_COMMON mesh/getoptwin.h)
endif()

add_library(gigamesh-common STATIC ${SOURCES_COMMON} ${HEADERS_COMMON})

target_compile_features(gigamesh-common PUBLIC cxx_std_17)

target_compile_options(gigamesh-common PUBLIC -DTHREADS -DNUM_THREADS=11 -DCOMP_USER=\"${COMP_USER}\" -DCOMP_DATE=\"${COMP_DATE}\" -DCOMP_GITHEAD=\"${COMP_GITHEAD}\" -DVERSION_PACKAGE=\"${VERSION_PACKAGE}\" -DCOMP_EDIT=\"${COMP_EDIT}\")

if(UNIX)
        target_link_libraries(gigamesh-common PRIVATE stdc++fs pthread)
elseif(MSVC)
    target_compile_options(gigamesh-common PUBLIC /permissive- -DNO_TIMER -DNOMINMAX)
    target_link_libraries(gigamesh-common PUBLIC Advapi32 Ws2_32)
elseif(MINGW)
    target_link_libraries(gigamesh-common PUBLIC ws2_32)
endif()

#-- INDIVIDUAL TOOLS--
add_executable(gigamesh-tolegacy mesh/gigamesh-tolegacy.cpp)
target_link_libraries(gigamesh-tolegacy PRIVATE gigamesh-common)

add_executable(gigamesh-clean mesh/gigamesh-clean.cpp)
target_link_libraries(gigamesh-clean PRIVATE gigamesh-common psalm)
target_compile_options(gigamesh-clean PRIVATE -DLIBPSALM)

add_executable(gigamesh-info mesh/gigamesh-info.cpp)
target_link_libraries(gigamesh-info PRIVATE gigamesh-common)

add_executable( gigamesh-featurevectors mesh/meshgeneratefeaturevectors25d.cpp)
target_link_libraries( gigamesh-featurevectors PRIVATE gigamesh-common)
target_include_directories( gigamesh-featurevectors PRIVATE ${CMAKE_SOURCE_DIR}/voxel
                                                          ${CMAKE_SOURCE_DIR}/mesh
                                                          ${CMAKE_SOURCE_DIR})


add_executable(meshgeneratefeaturevectors25d mesh/meshgeneratefeaturevectors25d.cpp ${SOURCES_COMMON} ${HEADERS_COMMON})
target_link_libraries(meshgeneratefeaturevectors25d)
target_compile_options(meshgeneratefeaturevectors25d PUBLIC -DCOMP_USER=\"${COMP_USER}\" -DCOMP_DATE=\"${COMP_DATE}\" -DCOMP_GITHEAD=\"${COMP_GITHEAD}\" -DVERSION_PACKAGE=\"${VERSION_PACKAGE}\" -DCOMP_EDIT=\"${COMP_EDIT}\")
target_include_directories( meshgeneratefeaturevectors25d PRIVATE ${CMAKE_SOURCE_DIR}/voxel
															  ${CMAKE_SOURCE_DIR}/mesh
															  ${CMAKE_SOURCE_DIR})
target_compile_features(meshgeneratefeaturevectors25d PUBLIC cxx_std_17)

if(UNIX)
	target_link_libraries(meshgeneratefeaturevectors25d PRIVATE stdc++fs pthread)
elseif(MSVC)
    target_compile_options(meshgeneratefeaturevectors25d PRIVATE /permissive- -DNO_TIMER -DNOMINMAX)
    target_link_libraries(meshgeneratefeaturevectors25d PRIVATE Advapi32 Ws2_32)
elseif(MINGW)
	target_link_libraries(meshgeneratefeaturevectors25d PRIVATE ws2_32)
endif()

add_executable(gigamesh-borders mesh/gigamesh-borders.cpp)
target_link_libraries(gigamesh-borders PRIVATE gigamesh-common)
