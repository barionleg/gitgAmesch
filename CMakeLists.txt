cmake_minimum_required(VERSION 3.10)

# CMAKEFile for building gigamesh

# note for building on Ubuntu, if g++-8 is not the default compiler, you have two options:
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

project(GigaMesh LANGUAGES CXX C)

set_property(GLOBAL PROPERTY USE_FOLDERS ON)

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

add_subdirectory(external)

add_subdirectory(core)

#set this via cmake <path to this CMakeLists.txt> -DGIGAMESH_GUI_ONLY=1
if(NOT GIGAMESH_GUI_ONLY)
add_subdirectory(clt)
endif()

#set this via cmake <path to this CMakeLists.txt> -DGIGAMESH_CLT_ONLY=1
if(NOT GIGAMESH_CLT_ONLY)
add_subdirectory(gui)
endif()
