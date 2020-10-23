
macro( add_lib projectname )

	target_link_libraries(${PROJECT_NAME} "${PROJECT_NAME_PREFIX}${projectname}")
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
