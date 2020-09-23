
macro( add_lib projectname )

	target_link_libraries(${PROJECT_NAME} "${PROJECT_NAME_PREFIX}${projectname}")
	target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/${projectname}")

endmacro()
