cmake_minimum_required(VERSION 3.13)

# CMAKEFile for building the command-line tools of gigamesh

# note for building the CLI for Ubuntu, if g++-8 is not the default compiler, you have two options:
# 1. Export CC and CXX before calling cmake:
# export CC=/usr/bin/gcc-8
# export CXX=/usr/bin/g++-8
#
# 2. set g++-8 as alternative permanently:
# sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 10
# sudo update-alternatives --install /usr/bin/g++ g++ /usr/bin/g++-8 10
# 
# 3. Tested alternative to 1, running cmake with:
# cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-8 -DCMAKE_CC_COMPILER=/usr/bin/gcc-8

#add_library(gigamesh-common STATIC ${SOURCES_COMMON} ${HEADERS_COMMON})

#target_compile_features(gigamesh-common PUBLIC cxx_std_17)

#target_compile_options(gigamesh-common PUBLIC -DTHREADS -DNUM_THREADS=11 -DCOMP_USER=\"${COMP_USER}\" -DCOMP_DATE=\"${COMP_DATE}\" -DCOMP_GITHEAD=\"${COMP_GITHEAD}\" -DVERSION_PACKAGE=\"${VERSION_PACKAGE}\" -DCOMP_EDIT=\"${COMP_EDIT}\")

#if(MSVC)
#	list(HEADERS_EXTRA getoptwin.h)
#endif()

#-- INDIVIDUAL TOOLS--
add_executable(gigamesh-tolegacy gigamesh-tolegacy.cpp)
target_link_libraries(gigamesh-tolegacy PRIVATE gigameshCore)

add_executable(gigamesh-clean gigamesh-clean.cpp)
target_link_libraries(gigamesh-clean PRIVATE gigameshCore psalm)
target_compile_options(gigamesh-clean PRIVATE -DLIBPSALM)

add_executable(gigamesh-info gigamesh-info.cpp)
target_link_libraries(gigamesh-info PRIVATE gigameshCore)

add_executable( gigamesh-featurevectors meshgeneratefeaturevectors25d.cpp)
target_link_libraries( gigamesh-featurevectors PRIVATE gigameshCore)
target_include_directories( gigamesh-featurevectors PRIVATE ${CMAKE_SOURCE_DIR}/voxel
                                                          ${CMAKE_SOURCE_DIR}/mesh
                                                          ${CMAKE_SOURCE_DIR})


#add_executable(meshgeneratefeaturevectors25d mesh/meshgeneratefeaturevectors25d.cpp ${SOURCES_COMMON} ${HEADERS_COMMON})
#target_link_libraries(meshgeneratefeaturevectors25d)
#target_compile_options(meshgeneratefeaturevectors25d PUBLIC -DCOMP_USER=\"${COMP_USER}\" -DCOMP_DATE=\"${COMP_DATE}\" -DCOMP_GITHEAD=\"${COMP_GITHEAD}\" -DVERSION_PACKAGE=\"${VERSION_PACKAGE}\" -DCOMP_EDIT=\"${COMP_EDIT}\")
#target_include_directories( meshgeneratefeaturevectors25d PRIVATE ${CMAKE_SOURCE_DIR}/voxel
#															  ${CMAKE_SOURCE_DIR}/mesh
#															  ${CMAKE_SOURCE_DIR})
#target_compile_features(meshgeneratefeaturevectors25d PUBLIC cxx_std_17)

#if(UNIX)
#	target_link_libraries(meshgeneratefeaturevectors25d PRIVATE stdc++fs pthread)
#elseif(MSVC)
#    target_compile_options(meshgeneratefeaturevectors25d PRIVATE /permissive- -DNO_TIMER -DNOMINMAX)
#    target_link_libraries(meshgeneratefeaturevectors25d PRIVATE Advapi32 Ws2_32)
#elseif(MINGW)
#	target_link_libraries(meshgeneratefeaturevectors25d PRIVATE ws2_32)
#endif()

add_executable(gigamesh-borders gigamesh-borders.cpp)
target_link_libraries(gigamesh-borders PRIVATE gigameshCore)
