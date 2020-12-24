cmake_minimum_required(VERSION 3.5)

#-march=armv7-a -mcpu=cortex-a5

project(tritonn LANGUAGES CXX C)
set(PROJECT_NAME_PREFIX TT_)

option(TRITONN_TESTS "Make tests" ON)
option(SIMPLETEST_USING_SIMPLEARGS "Simpletest using SimpleArgs" ON)
option(TRITONN_X64 "Make 64 bits version" OFF)

#
if(NOT DEFINED PROJECT_VERSION_MAJOR)
	set(PROJECT_VERSION_MAJOR 0)
	message(AUTHOR_WARNING "Set major version: ${PROJECT_VERSION_MAJOR}")
endif()

if(NOT DEFINED PROJECT_VERSION_MINOR)
	set(PROJECT_VERSION_MINOR 18)
	message(AUTHOR_WARNING "Set minor version: ${PROJECT_VERSION_MINOR}")
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
set_project_version("src/tritonn_version.h.in")
include_directories("${CMAKE_BINARY_DIR}/src")
#
add_subdirectory(src)

# Библиотеки PThread и OpenSSL
#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
#target_link_libraries(${TARGET} PRIVATE Threads::Threads)
# -lpthread

find_package(OpenSSL REQUIRED)
if(OPENSSL_FOUND)
	message(STATUS "Using OpenSSL ver ${OPENSSL_VERSION} in '${OPENSSL_INCLUDE_DIR}'")
	include_directories(${OPENSSL_INCLUDE_DIR})
else()
	message(FATAL_ERROR "Failed to find openssl")
	return()
endif()

message(STATUS "Host: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Version: ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_BUILD}.${PROJECT_VERSION_HASH} ${PROJECT_VERSION_DATE} ${PROJECT_VERSION_TIME} (${PROJECT_VERSION_UNIXTIME})")
message(STATUS "CMAKE_SOURCE_DIR: ${CMAKE_SOURCE_DIR}")
message(STATUS "CMAKE_BINARY_DIR: ${CMAKE_BINARY_DIR}")
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")


