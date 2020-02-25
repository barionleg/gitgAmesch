cmake_minimum_required(VERSION 3.10)

project(libalg LANGUAGES CXX)

set(ALG_SOURCES
	src/ap.cpp
	src/corr.cpp
	src/ialglib.cpp
	src/ftbase.cpp
	src/fft.cpp
	src/conv.cpp
	src/ablas.cpp
	src/ablasf.cpp)


add_library(alg STATIC ${ALG_SOURCES} ${ALG_HEADERS})

target_include_directories(alg PUBLIC
							$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>
							$<INSTALL_INTERFACE:include>
							PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/src)

target_include_directories(alg PRIVATE ${CMAKE_CURRENT_SOURCE_DIR}/include/alg)
target_compile_features(alg PRIVATE cxx_std_17)
#set_target_properties(alg PROPERTIES PUBLIC_HEADER )

install(TARGETS alg
		ARCHIVE DESTINATION lib
		LIBRARY DESTINATION lib
		PUBLIC_HEADER DESTINATION include/alg)
