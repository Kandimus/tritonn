
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
