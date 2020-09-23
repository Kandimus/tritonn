cmake_minimum_required(VERSION 3.5)

#-march=armv7-a -mcpu=cortex-a5

project(tritonn LANGUAGES CXX C)
set(PROJECT_NAME_PREFIX TT_)

option(TRITONN_TESTS "Make tests" ON)

if(NOT DEFINED TRITONN_VER_MAJOR)
	set(TRITONN_VER_MAJOR 0)
	message(AUTHOR_WARNING "Set major version: ${TRITONN_VER_MAJOR}")
endif()

if(NOT DEFINED TRITONN_VER_MINOR)
	set(TRITONN_VER_MINOR 17)
	message(AUTHOR_WARNING "Set minor version: ${TRITONN_VER_MINOR}")
endif()

set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_CURRENT_SOURCE_DIR}/cmake")
include(UsefulMacros)

set(CMAKE_INSTALL_PREFIX "~/tritonn")

set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "-lstdc++ -std=c++14 -m32 -Wall -Wfloat-conversion")
set(CMAKE_C_FLAGS "-std=c99 -m32")
set(CMAKE_LIBRARY_PATH "/usr/lib/i386-linux-gnu")

add_subdirectory(src)


# Библиотеки PThread и OpenSSL
#set(THREADS_PREFER_PTHREAD_FLAG ON)
#find_package(Threads REQUIRED)
#target_link_libraries(${TARGET} PRIVATE Threads::Threads)

#find_package(OpenSSL REQUIRED)
##target_link_libraries(${TARGET} PRIVATE libssl.so libcrypto.so)

#if( OpenSSL_FOUND )
#    include_directories(${OPENSSL_INCLUDE_DIRS})
#    message(STATUS "Using OpenSSL ${OPENSSL_VERSION}")
#else()
#    message(FATAL_ERROR "OpenSLL not found.")
#endif()

message(STATUS "Host: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Version: ${TRITONN_VER_MAJOR}.${TRITONN_VER_MINOR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")


