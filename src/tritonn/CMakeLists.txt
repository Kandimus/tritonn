
# Sources
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE files_h ${PROJECT_SOURCE_DIR}/*.h)

add_executable(tritonn ${files_cpp} ${files_h})

target_compile_definitions(tritonn PRIVATE LOG_APP_NAME="tritonn")

target_link_libraries(tritonn shared)
target_link_libraries(tritonn open62541)

# Генерация исходного кода
add_custom_target(run_verinc ALL
	COMMAND ${CMAKE_CURRENT_SOURCE_DIR}/../../verinc  -M ${TRITONN_VER_MAJOR} -m ${TRITONN_VER_MINOR} -p 0 -i -F ${CMAKE_CURRENT_SOURCE_DIR}/build.incver -a \"\" -P TRITONN > ${CMAKE_CURRENT_SOURCE_DIR}/tritonn_version.h
	BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/tritonn_version.h & sleep 0.5
	COMMENT "Generating build version")
add_dependencies(tritonn run_verinc)

