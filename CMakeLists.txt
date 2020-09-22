cmake_minimum_required(VERSION 3.5)

#-march=armv7-a -mcpu=cortex-a5

project(tritonn_project LANGUAGES CXX C)

option(TRITONN_TESTS "Make tests" ON)

if(NOT DEFINED TRITONN_VER_MAJOR)
	set(TRITONN_VER_MAJOR 0)
	message(AUTHOR_WARNING "Set major version: ${TRITONN_VER_MAJOR}")
endif()

if(NOT DEFINED TRITONN_VER_MINOR)
	set(TRITONN_VER_MINOR 17)
	message(AUTHOR_WARNING "Set minor version: ${TRITONN_VER_MINOR}")
endif()

set(CMAKE_INSTALL_PREFIX "~/tritonn")



set(CXX_STANDARD 14)
set(CXX_STANDARD_REQUIRED YES)
set(CMAKE_CXX_FLAGS "-lstdc++ -std=c++14 -m32 -Wall -Wfloat-conversion")
set(CMAKE_C_FLAGS "-std=c99 -m32")
set(CMAKE_LIBRARY_PATH "/usr/lib/i386-linux-gnu")

#target_compile_options(${TARGET} PRIVATE "-Wall -Wfloat-conversion -m32")
#set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")

add_subdirectory(src)

#include_directories(src/shared)
#include_directories(src/tritonn)
#include_directories(src/open62541)

#file(GLOB_RECURSE src_shared      src/shared/*.cpp src/shared/*.h src/shared/*.c)
#file(GLOB_RECURSE src_tritonn     src/tritonn/*.cpp src/tritonn/*.h src/tritonn/*.c)
#file(GLOB_RECURSE src_open62541   src/open62541/*.cpp src/open62541/*.h src/open62541/*.c)

#message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
#add_executable(${TARGET} ${src_open62541} ${src_shared} ${src_tritonn})


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

#target_link_libraries(${TARGET} ${OpenSSL_LIBRARIES})

################################################################

#target_link_libraries(${TARGET} PRIVATE crypto ssl)
#target_link_libraries(${TARGET} PRIVATE pthread)

#install(TARGETS ${TARGET} DESTINATION ~/tritonn/)

# Генерация исходного кода
#add_custom_target(run_verinc ALL
#	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/verinc  -M ${TRITONN_VER_MAJOR} -m ${TRITONN_VER_MINOR} -p 0 -i -F ${CMAKE_CURRENT_SOURCE_DIR}/src/tritonn/build.incver -a \"\" -P TRITONN > ${CMAKE_CURRENT_SOURCE_DIR}/src/tritonn/tritonn_version.h
#	BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/src/tritonn/tritonn_version.h & sleep 1s
#	COMMENT "Generating build version")
#add_dependencies(${TARGET} run_verinc)

message(STATUS "Host: ${CMAKE_SYSTEM_PROCESSOR}")
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Version: ${TRITONN_VER_MAJOR}.${TRITONN_VER_MINOR}")


