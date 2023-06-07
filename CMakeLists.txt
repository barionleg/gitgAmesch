cmake_minimum_required(VERSION 3.10)
set(CMAKE_OSX_DEPLOYMENT_TARGET "10.15" CACHE STRING "Minimum OS X deployment version")

# CMAKEFile for building gigamesh

# note: building GigaMesh requires a compiler which supports C++17 features. So please
# make sure that your compiler has the following version or newer:
# gcc        : 8
# clang      : 7
# msvc stl   : 19.14
# apple clang: 11.0.0
#
# on ubuntu for example, you have the following options to set the gcc8 compiler, assuming it is in usr/bin/:
# 1. Running cmake with:
# cmake .. -DCMAKE_CXX_COMPILER=/usr/bin/g++-8 -DCMAKE_CC_COMPILER=/usr/bin/gcc-8
#
# 2. Export CC and CXX before calling cmake:
# export CC=/usr/bin/gcc-8
# export CXX=/usr/bin/g++-8
#
# 3. Set g++-8 as alternative permanently:
# sudo update-alternatives --install /usr/bin/gcc gcc /usr/bin/gcc-8 90 --slave /usr/bin/g++ g++ /usr/bin/g++-8 --slave /usr/bin/gcov gcov /usr/bin/gcov-8
#


project(GigaMesh LANGUAGES CXX C
			DESCRIPTION "GigaMesh is a software Framework for processing large meshes.")

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED True)



option(BUILD_CLI "Build the GigaMesh-CLI tools" ON)
option(BUILD_GUI "Build GigaMesh-GUI" ON)
option(ENABLE_TESTS "Build tests" ON)

#--------- get git version and other useful information ---------
string(TIMESTAMP COMP_DATE "%d.%b.%y")
execute_process(COMMAND git rev-parse HEAD
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE COMP_GITHEAD)
string(STRIP "${COMP_GITHEAD}" COMP_GITHEAD)

execute_process(COMMAND git log -1 --format=%ci
                WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
                OUTPUT_VARIABLE VERSION_PACKAGE)
#set(COMP_EDIT IWR\ Open\ Source)
set(COMP_EDIT MLU\ Open\ Source)
string(STRIP "${VERSION_PACKAGE}" VERSION_PACKAGE)
string(SUBSTRING "${VERSION_PACKAGE}" 2 9 VERSION_PACKAGE)
string(REPLACE "-" "" VERSION_PACKAGE "${VERSION_PACKAGE}")
string(STRIP "${VERSION_PACKAGE}" VERSION_PACKAGE)

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
#-------------------------------------------------------------

add_subdirectory(external)
add_subdirectory(core)

#disable CLI build by setting -DBUILD_CLI=OFF, or editing the variable in CMakeCache.txt
if(BUILD_CLI)
add_subdirectory(cli)
endif()

#disable GUI build by setting -DBUILD_GUI=OFF, or editing the variable in CMakeCache.txt
if(BUILD_GUI)
add_subdirectory(gui)
endif()

#disable test build by setting -DENABLE_TESTS=OFF, or editing the variable in CMakeCache.txt
if(ENABLE_TESTS)
	include(CTest)
	add_subdirectory(tests)
endif()

install(FILES	CHANGELOG
				README.md
				README_LIBPSALM.txt
				COPYING.txt
				LICENSE.txt
		DESTINATION share/doc/gigamesh)

#CPack, see: https://cmake.org/cmake/help/v3.10/module/CPack.html
include(InstallRequiredSystemLibraries)
#set variables before include(CPack) to override defaults of CPack
set(CPACK_RESOURCE_FILE_LICENSE "${CMAKE_CURRENT_SOURCE_DIR}/COPYING.txt")
set(CPACK_PACKAGE_VERSION "${VERSION_PACKAGE}")
set(CPACK_SOURCE_STRIP_FILES TRUE)
set(CPACK_STRIP_FILES "bin/gigamesh;bin/gigamesh-tolegacy;bin/gigamesh-clean;bin/gigamesh-info;bin/gigamesh-featurevectors;bin/gigamesh-borders")

set(CPACK_DEBIAN_PACKAGE_MAINTAINER "Hubert Mara <hubert.mara@iwr.uni-heidelberg.de>")
set(CPACK_DEBIAN_PACKAGE_DEPENDS "libc6 (>= 2.14), libgcc1 (>= 1:3.0), qtbase5-dev(>= 5.5),libqt5core5a (>= 5), libqt5gui5 (>= 5) | libqt5gui5-gles (>= 5), libqt5opengl5 (>= 5) | libqt5opengl5-gles (>= 5), libstdc++6 (>= 5), libtiff6 (>= 4), inkscape(>=0.92)")
set(CPACK_DEBIAN_PACKAGE_HOMEPAGE "https://www.gigamesh.eu")
set(CPACK_DEBIAN_PACKAGE_SECTION "science")

#set(CPACK_PACKAGE_FILE_NAME "${CPACK_PACKAGE_NAME}-${CPACK_PACKAGE_VERSION}-${CPACK_SYSTEM_NAME}")

include(CPack)

