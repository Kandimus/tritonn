if(WIN32)
	set(CMAKE_FIND_LIBRARY_SUFFIXES "" ".dll" ".lib" ".a")
endif()

string( TOUPPER ${CMAKE_BUILD_TYPE} CMAKE_BUILD_TYPEUP)

if(CMAKE_BUILD_TYPEUP STREQUAL DEBUG AND WIN32)
	set(BICYCLE_DEBUG_POSTFIX "d")
endif()


if(NOT SPEC OR NOT HOST)
	if(CMAKE_SYSTEM_NAME STREQUAL "Linux")
		set(HOST linux)
		if(CMAKE_SYSTEM_PROCESSOR STREQUAL "i686")
			set(SPEC linux)
		elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "armv")
			set(SPEC arm)
			add_compile_definitions(TRITONN_YOCTO)
		elseif(CMAKE_SYSTEM_PROCESSOR STREQUAL "x86_64")
			set(SPEC linux)
		endif()
	elseif(CMAKE_SYSTEM_NAME STREQUAL "Windows")
		set(HOST "win32")
		if(MSVC)
			if(CMAKE_CL_64)
				set(SPEC "msvc")
			else()
				set(SPEC "msvc32")
			endif()
		else()
			set(SPEC "mingw")
		endif()
	endif()
endif()

### MACRO #########################################################################################

macro( add_lib projectname )

	target_link_libraries(${PROJECT_NAME} PRIVATE "${PROJECT_NAME_PREFIX}${projectname}")
	target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/${projectname}")

endmacro()


macro( compile_submodule submodulename ADD_DIRECTORIES ADD_DEFININITIONS)
	project(${PROJECT_NAME_PREFIX}${submodulename})

	file(GLOB submodule_cpp ${CMAKE_SOURCE_DIR}/src/${submodulename}/*.c
							${CMAKE_SOURCE_DIR}/src/${submodulename}/*.cpp)
	file(GLOB submodule_h   ${CMAKE_SOURCE_DIR}/src/${submodulename}/*.h)

	add_library(${PROJECT_NAME} STATIC ${submodule_cpp} ${submodule_h})

	target_include_directories(${PROJECT_NAME} PUBLIC ${ADD_DIRECTORIES})

	target_compile_definitions(${PROJECT_NAME} PUBLIC ${ADD_DEFININITIONS})

endmacro()


macro( set_project_version FILE_IN)
	set(PROJECT_VERSION_BUILD "0")
	set(PROJECT_VERSION_HASH "undef")
	set(PROJECT_VERSION_DATE "1900-01-01")
	set(PROJECT_VERSION_TIME "00:00:00")
	set(PROJECT_VERSION_UNIXTIME "0")

	find_package(Git)
	if(GIT_FOUND)
		# HASH
		execute_process(COMMAND ${GIT_EXECUTABLE} describe --tags --long --abbrev=8
					WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
					OUTPUT_VARIABLE GIT_HASH
					OUTPUT_STRIP_TRAILING_WHITESPACE)
		if( GIT_HASH )
			set(PROJECT_VERSION_HASH "")
			set(VER_POS -1)
			set(VER_POS_HASH -1)
			string( FIND ${GIT_HASH} "g" VER_POS)
			math(EXPR VER_POS_HASH "${VER_POS} + 1")
			string( SUBSTRING ${GIT_HASH} ${VER_POS_HASH} "255" PROJECT_VERSION_HASH)
			#string( REGEX REPLACE "\\-g" "." TRITONN_VER_GIT ${TRITONN_VER_GIT} )
			#string( REGEX REPLACE "\\-" "." PROJECT_VERSION_HASH ${PROJECT_VERSION_HASH} )
			math(EXPR VER_POS "${VER_POS} - 1")
			string( SUBSTRING ${GIT_HASH} 0 ${VER_POS} GIT_HASH)
			string( FIND ${GIT_HASH} "-" VER_POS)
			math(EXPR VER_POS "${VER_POS} + 1")
			string( SUBSTRING ${GIT_HASH} ${VER_POS} "255" PROJECT_VERSION_BUILD)
		endif()

		# datetime
		execute_process(COMMAND ${GIT_EXECUTABLE} show -s --format=%ci HEAD
					WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
					OUTPUT_VARIABLE GIT_DATETIME
					OUTPUT_STRIP_TRAILING_WHITESPACE)
		if( GIT_DATETIME )
			set(PROJECT_VERSION_DATE "")
			set(PROJECT_VERSION_TIME "")
			set(VER_POS -1)
			string( FIND ${GIT_DATETIME} " " VER_POS)
			string( SUBSTRING ${GIT_DATETIME} 0 ${VER_POS} PROJECT_VERSION_DATE)
			math(EXPR VER_POS "${VER_POS} + 1")
			string( SUBSTRING ${GIT_DATETIME} ${VER_POS} 255 GIT_DATETIME)
			string( FIND ${GIT_DATETIME} " " VER_POS)
			string( SUBSTRING ${GIT_DATETIME} 0 ${VER_POS} PROJECT_VERSION_TIME)
		endif()

		# unixtime
		execute_process(COMMAND ${GIT_EXECUTABLE} show -s --format=%ct HEAD
					WORKING_DIRECTORY ${PROJECT_SOURCE_DIR}
					OUTPUT_VARIABLE GIT_UNIXTIME
					OUTPUT_STRIP_TRAILING_WHITESPACE)
		if( GIT_UNIXTIME )
			set(PROJECT_VERSION_UNIXTIME ${GIT_UNIXTIME})
		endif()
	endif()

	set(VER_POS -1)
	string( FIND ${FILE_IN} ".in" VER_POS)
	string( SUBSTRING ${FILE_IN} 0 ${VER_POS} FILE_OUT)
	configure_file("${CMAKE_SOURCE_DIR}/${FILE_IN}" "${CMAKE_BINARY_DIR}/${FILE_OUT}")
endmacro()

function (ListToString result delim)
	list(GET ARGV 2 temp)
	math(EXPR N "${ARGC}-1")
	foreach(IDX RANGE 3 ${N})
		list(GET ARGV ${IDX} STR)
		set(temp "${temp}${delim}${STR}")
	endforeach()
	set(${result} "${temp}" PARENT_SCOPE)
endfunction(ListToString)

macro( compile_proto PROJECTNAME)
	file( GLOB_RECURSE protos "*.proto" )
	file( GLOB_RECURSE protosin "*.proto.in" )

	foreach( proto ${exclude_proto} )
		list( REMOVE_ITEM protos ${proto} )
	endforeach()

	list(APPEND protos ${add_proto})

	foreach( protoin ${exclude_protoin} )
		list( REMOVE_ITEM protosin ${protoin} )
	endforeach()

	list(APPEND protosin ${add_protoin})

	foreach(protoin_item ${protosin})
		get_filename_component(protoin_file_name ${protoin_item} NAME_WE)

		if(PROTOBUF_FULL_LIB)
			set(PROTOBUF_FULL_LIB_STRING "")
		else()
			set(PROTOBUF_FULL_LIB_STRING "option optimize_for = LITE_RUNTIME;")
		endif()

		set(dest_protoin_file ${${PROJECTNAME}_BINARY_DIR}/${protoin_file_name}.proto)
		configure_file(${protoin_item} ${dest_protoin_file} @ONLY)
		set_property(SOURCE ${dest_protoin_file} PROPERTY GENERATED ON)
		list(APPEND protos ${dest_protoin_file})
	endforeach()

	list(LENGTH protos protos_len)

	if(protos_len GREATER 0)
#		list( APPEND srcs ${protos})
		foreach(proto_item ${protos})
			get_filename_component(proto_item_NAME ${proto_item} NAME_WE)
			get_filename_component(proto_item_DIR ${proto_item} PATH)

			set(protos_out ${${PROJECTNAME}_BINARY_DIR}/${proto_item_NAME}.pb.cc ${${PROJECTNAME}_BINARY_DIR}/${proto_item_NAME}.pb.h)
			get_property(a3 SOURCE ${proto_item} PROPERTY GENERATED)

			list(APPEND proto_path "--proto_path=${PROTOBUF_INCLUDE_DIR}")

			if (a3)
				list(APPEND proto_path "--proto_path=${proto_item_DIR}" "--proto_path=${${PROJECTNAME}_BINARY_DIR}" "--proto_path=${${PROJECTNAME}_SOURCE_DIR}")
				foreach(lib ${libs} )
					if(${lib} MATCHES ${PROJECT_NAME_GLOBAL} )
						list(APPEND proto_path "--proto_path=${${lib}_BINARY_DIR}" "--proto_path=${${lib}_SOURCE_DIR}")
					endif()
				endforeach()
			else()
				set(proto_path "--proto_path=${proto_item_DIR}")
			endif()

			list(APPEND protos_out_all ${protos_out})
			list(REMOVE_DUPLICATES proto_path)
			list(APPEND srcs ${protos_out})
			set_source_files_properties(${protos_out} PROPERTIES GENERATED TRUE)

			#ListToString(proto_path_str, " ", ${proto_path})
			#string(REPLACE ";" " " proto_path_str "${proto_path}")
			#string(REPLACE ";" " " protos_out_str "${protos_out}")

			message(STATUS "PROTOBUF: ======================================")
			message(STATUS "COMMAND: ${PROTOBUF_COMPILER} ${proto_path_str} --cpp_out=\"${${PROJECTNAME}_BINARY_DIR}\" ${proto_item}")
			message(STATUS "proto_item: ${proto_item}")
			message(STATUS "protos_out: ${protos_out}")

			add_custom_command(
				COMMAND ${PROTOBUF_COMPILER}
				ARGS ${proto_path} --cpp_out="${${PROJECTNAME}_BINARY_DIR}" ${proto_item}
				DEPENDS ${proto_item}
				OUTPUT ${protos_out}
			)
		endforeach()

		add_custom_target(${PROJECTNAME}_protobuf_autogen ALL DEPENDS ${protos_out_all})
	endif()

endmacro()
