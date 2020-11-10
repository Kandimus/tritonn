project(${PROJECT_NAME_PREFIX}ttterm)

# Sources
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp
							${PROJECT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE files_h	${PROJECT_SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME} ${files_cpp} ${files_h})

# OUTPUTNAME
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ttterm")

# LIBRARIES
target_link_libraries(${PROJECT_NAME} PRIVATE crypto ssl pthread ncurses)

add_lib(shared)

target_include_directories(${PROJECT_NAME} PRIVATE "${PROJECT_BINARY_DIR}")

# Генерация исходного кода
#${CMAKE_CURRENT_SOURCE_DIR}/ttterm_version.h
add_custom_target(ttterm_run_verinc ALL
	COMMAND ${CMAKE_SOURCE_DIR}/verinc  -M ${TRITONN_VER_MAJOR} -m ${TRITONN_VER_MINOR} -p 0 -i -F ${PROJECT_SOURCE_DIR}/build.incver -a \"\" -P TTTERM > ${PROJECT_BINARY_DIR}/ttterm_version.h
	BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/ttterm_version.h & sleep 0.5
	COMMENT "Generating ttterm build version")
add_dependencies(${PROJECT_NAME} ttterm_run_verinc)

# INSTALL
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
