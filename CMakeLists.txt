cmake_minimum_required(VERSION 3.10)

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
add_subdirectory(clt)
add_subdirectory(gui)
