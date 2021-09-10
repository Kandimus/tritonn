#
# Этот файл должен находиться в корне тулчейна!
#

# this one is important
SET(CMAKE_SYSTEM_NAME Linux)
SET(CMAKE_SYSTEM_PROCESSOR arm)
#this one not so much
SET(CMAKE_SYSTEM_VERSION 1)

#if(${TOOLCHAIN_ENABLE} EQUAL 0)
	# Получаем путь до директории toolchain.
#	SET(TOOLCHAIN_ENABLE 2)
#if(DEFINED ${CMAKE_TOOLCHAIN_FILE})
#	STRING(REPLACE "/toolchain.cmake" "" TOOLCHAIN_PATH ${CMAKE_TOOLCHAIN_FILE})
#	message(STATUS "Toolchain file: ${CMAKE_TOOLCHAIN_FILE}")
#	message(STATUS "Toolchain path: ${TOOLCHAIN_PATH}")
#endif()

SET(TOOLCHAIN_PATH "/opt/gcc-linaro-7.5.0-2019.12-x86_64_arm-linux-gnueabihf")

# specify the cross compiler
SET(CMAKE_C_COMPILER   ${TOOLCHAIN_PATH}/bin/arm-linux-gnueabihf-gcc)
SET(CMAKE_CXX_COMPILER ${TOOLCHAIN_PATH}/bin/arm-linux-gnueabihf-g++)

# where is the target environment
#SET(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN_PATH}/arm-linux-gnueabihf/libc/usr)
#SET(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN_PATH}/arm-linux-gnueabihf)
SET(CMAKE_FIND_ROOT_PATH  ${TOOLCHAIN_PATH})

# search for programs in the build host directories
SET(CMAKE_FIND_ROOT_PATH_MODE_PROGRAM NEVER)
# for libraries and headers in the target directories
SET(CMAKE_FIND_ROOT_PATH_MODE_LIBRARY BOTH)
SET(CMAKE_FIND_ROOT_PATH_MODE_INCLUDE BOTH)

set(CMAKE_FIND_LIBRARY_SUFFIXES ".a")
#set(CMAKE_EXE_LINKER_FLAGS "-static-libgcc -static-libstdc++")



