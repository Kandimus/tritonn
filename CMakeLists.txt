cmake_minimum_required(VERSION 3.5)

#-march=armv7-a -mcpu=cortex-a5

project(tritonn LANGUAGES CXX C)
set(PROJECT_NAME_PREFIX TT_)

option(TRITONN_TESTS "Make tests" ON)
option(SIMPLETEST_USING_SIMPLEARGS "Simpletest using SimpleArgs" ON)
option(TRITONN_X64 "Make 64 bits version" OFF)

#
if(NOT DEFINED TRITONN_VER_MAJOR)
	set(TRITONN_VER_MAJOR 0)
	message(AUTHOR_WARNING "Set major version: ${TRITONN_VER_MAJOR}")
endif()

if(NOT DEFINED TRITONN_VER_MINOR)
	set(TRITONN_VER_MINOR 18)
	message(AUTHOR_WARNING "Set minor version: ${TRITONN_VER_MINOR}")
endif()

#
add_compile_definitions(TRITONN_SOURCE_PATH="${CMAKE_SOURCE_DIR}/src/")

#
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")

#
include(UsefulMacros)

#
set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "-lstdc++ -std=c++14 -Wall -Wfloat-conversion")
set(CMAKE_C_FLAGS "-std=c99")

if(${CMAKE_SIZEOF_VOID_P} STREQUAL 8)
	if(NOT TRITONN_X64)
		set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} -m32")
		set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -m32")

		set(CMAKE_LIBRARY_PATH "/usr/lib/i386-linux-gnu")
	endif()
endif()

#generate version
find_package(Git)
if(GIT_FOUND)
	execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --dirty --long
				WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
				OUTPUT_VARIABLE TRITONN_VER_GIT
				OUTPUT_STRIP_TRAILING_WHITESPACE)
	if( TRITONN_VER_GIT )
		set(TRITONN_VER_BUILD "")
		set(VER_POS -1)
		string( FIND ${TRITONN_VER_GIT} "g" VER_POS)
		math(EXPR VER_POS "${VER_POS} + 1")
		string( SUBSTRING ${TRITONN_VER_GIT} ${VER_POS} "255" TRITONN_VER_BUILD)
		#string( REGEX REPLACE "\\-g" "." TRITONN_VER_GIT ${TRITONN_VER_GIT} )
		string( REGEX REPLACE "\\-" "." TRITONN_VER_GIT ${TRITONN_VER_GIT} )
	else()
		set(TRITONN_VER_GIT "undef")
	endif()
else()
	set(TRITONN_VER_GIT "00000000")
endif()

#
add_subdirectory(src)

# Библиотеки PThread и OpenSSL
#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
#target_link_libraries(${TARGET} PRIVATE Threads::Threads)

#find_package(OpenSSL REQUIRED)
##target_link_libraries(${TARGET} PRIVATE libssl.so libcrypto.so)
# -lpthread

#if( OpenSSL_FOUND )
#    include_directories(${OPENSSL_INCLUDE_DIRS})
#    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
#else()
#    message(FATAL_ERROR "OpenSLL not found.")
#endif()

message(STATUS "Host: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Version: ${TRITONN_VER_MAJOR}.${TRITONN_VER_MINOR}.${TRITONN_VER_BUILD}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")


