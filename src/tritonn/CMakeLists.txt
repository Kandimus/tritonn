project(${PROJECT_NAME_PREFIX}tritonn)

# Sources
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE files_c ${PROJECT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE files_h ${PROJECT_SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME} ${files_cpp} ${files_c} ${files_h})

add_lib(shared)
add_lib(open62541)

# Генерация исходного кода
add_custom_target(run_verinc ALL
	COMMAND ${CMAKE_SOURCE_DIR}/verinc  -M ${TRITONN_VER_MAJOR} -m ${TRITONN_VER_MINOR} -p 0 -i -F ${CMAKE_CURRENT_SOURCE_DIR}/build.incver -a \"\" -P TRITONN > ${CMAKE_CURRENT_SOURCE_DIR}/tritonn_version.h
	BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/tritonn_version.h & sleep 0.5
	COMMENT "Generating build version")
add_dependencies(${PROJECT_NAME} run_verinc)

