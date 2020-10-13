cmake_minimum_required(VERSION 3.5)

#-march=armv7-a -mcpu=cortex-a5

project(tritonn LANGUAGES CXX C)
set(PROJECT_NAME_PREFIX TT_)

option(TRITONN_TESTS "Make tests" ON)

#
if(NOT DEFINED TRITONN_VER_MAJOR)
	set(TRITONN_VER_MAJOR 0)
	message(AUTHOR_WARNING "Set major version: ${TRITONN_VER_MAJOR}")
endif()

if(NOT DEFINED TRITONN_VER_MINOR)
	set(TRITONN_VER_MINOR 17)
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
set(CMAKE_CXX_FLAGS "-lstdc++ -std=c++14 -m32 -Wall -Wfloat-conversion")
set(CMAKE_C_FLAGS "-std=c99 -m32")
set(CMAKE_LIBRARY_PATH "/usr/lib/i386-linux-gnu")

<<<<<<< HEAD
message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
add_executable(${TARGET} ${src_open62541} ${src_shared} ${src_tritonn} )
||||||| merged common ancestors
message(STATUS "System name: ${CMAKE_SYSTEM_NAME}")
if(NOT DEFINED ${TOOLCHAIN_ENABLE})
	message(STATUS "Toolchain: none")
else()
	message(STATUS "Toolchain: ${CMAKE_TOOLCHAIN_FILE}")
endif()
=======
#
add_subdirectory(src)
>>>>>>> Core-5

<<<<<<< HEAD

set(CMAKE_CXX_FLAGS -lstdc++)
set(CMAKE_C_FLAGS -std=c99)


target_compile_options(${TARGET} PRIVATE -std=c++11 -Wall -Wfloat-conversion -m32)
set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
||||||| merged common ancestors
#if(NOT CMAKE_SYSTEM_NAME STREQUAL "Yocto")

#endif()
add_executable(${TARGET} ${src_open62541} ${src_shared} ${src_tritonn} )

#target_compile_options(${TARGET} PRIVATE -std=c++11 -Wall -Wfloat-conversion)
target_compile_options(${TARGET} PRIVATE -Wall -Wfloat-conversion)
=======
>>>>>>> Core-5

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

<<<<<<< HEAD
#target_link_libraries(${TARGET} ${OpenSSL_LIBRARIES})
target_link_libraries(${TARGET} PRIVATE crypto ssl)
target_link_libraries(${TARGET} PRIVATE pthread)

install(TARGETS ${TARGET} DESTINATION ~/tritonn/)

# Определение типа хоста, установка необходимых флагов компиляции
set(CMAKE_LIBRARY_PATH "/usr/lib/i386-linux-gnu")
set(CMAKE_C_FLAGS -m32)
set(CMAKE_CXX_FLAGS -m32)

# Генерация исходного кода
add_custom_target(run_verinc ALL
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/verinc  -M ${TRITONN_VER_MAJOR} -m ${TRITONN_VER_MINOR} -p 0 -i -F ${CMAKE_CURRENT_SOURCE_DIR}/src/tritonn/build.incver -a \"\" -P TRITONN > ${CMAKE_CURRENT_SOURCE_DIR}/src/tritonn/tritonn_version.h
	BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/src/tritonn/tritonn_version.h & sleep 1s
	COMMENT "Generating build version")
add_dependencies(${TARGET} run_verinc)

message(STATUS "Host: ${CMAKE_SYSTEM_PROCESSOR}")
||||||| merged common ancestors
#target_link_libraries(${TARGET} ${OpenSSL_LIBRARIES})
target_link_libraries(${TARGET} PRIVATE crypto ssl)
target_link_libraries(${TARGET} PRIVATE pthread)

install(TARGETS ${TARGET} DESTINATION ~/tritonn/)

# Определение типа хоста, установка необходимых флагов компиляции
message(STATUS "Host: ${CMAKE_SYSTEM_PROCESSOR}")
if(${CMAKE_SYSTEM_PROCESSOR} STREQUAL "x86_64")
	set(CMAKE_LIBRARY_PATH "/usr/lib/i386-linux-gnu")
	set(CMAKE_C_FLAGS -m32)
	set(CMAKE_CXX_FLAGS -m32)
	set(CMAKE_LIBRARY_PATH "/usr/lib/i386-linux-gnu")
	set_target_properties(${TARGET} PROPERTIES COMPILE_FLAGS "-m32" LINK_FLAGS "-m32")
else()
	message(STATUS "Library: STATIC")
#	set_target_properties(${TARGET} PROPERTIES LINK_SEARCH_START_STATIC 1)
#	set_target_properties(${TARGET} PROPERTIES LINK_SEARCH_END_STATIC 1)
endif()
set(CMAKE_CXX_FLAGS -lstdc++)
set(CMAKE_C_FLAGS -std=c99)


if(NOT DEFINED TOOLCHAIN_ENABLE)
	# Генерация исходного кода
	add_custom_target(run_verinc ALL
		COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/verinc  -M 0 -m 17 -p 0 -i -F ${CMAKE_CURRENT_SOURCE_DIR}/src/tritonn/build.incver -a \"\" -P TRITONN > ${CMAKE_CURRENT_SOURCE_DIR}/src/tritonn/tritonn_version.h
		BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/src/tritonn/tritonn_version.h & sleep 1s
		COMMENT "Generating build version")
	add_dependencies(${TARGET} run_verinc)
endif()

message(STATUS "TOOLCHAIN_ENABLE ${TOOLCHAIN_ENABLE}")
=======
message(STATUS "Host: ${CMAKE_SYSTEM_PROCESSOR}")
>>>>>>> Core-5
message(STATUS "C compiler: ${CMAKE_C_COMPILER}")
message(STATUS "CXX compiler: ${CMAKE_CXX_COMPILER}")
message(STATUS "Version: ${TRITONN_VER_MAJOR}.${TRITONN_VER_MINOR}")
message(STATUS "CMAKE_CURRENT_SOURCE_DIR: ${CMAKE_CURRENT_SOURCE_DIR}")
message(STATUS "CMAKE_INSTALL_PREFIX: ${CMAKE_INSTALL_PREFIX}")


