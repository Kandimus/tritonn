project(${PROJECT_NAME_PREFIX}tritonn)

# Sources
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp
									 ${PROJECT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE files_h ${PROJECT_SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME} ${files_cpp} ${files_h})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_SOURCE_DIR}/bin/modbus.xml
				  ${CMAKE_SOURCE_DIR}/bin/systemevent.xml
				  ${CMAKE_SOURCE_DIR}/bin/systemtext.xml
				  ${CMAKE_SOURCE_DIR}/bin/tritonn.conf
		  DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/cert DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/conf DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/diag DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/ftp DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/log DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/www DESTINATION ${CMAKE_INSTALL_PREFIX})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "tritonn")

add_lib(shared)
add_lib(open62541)
add_lib(simpleargs)

# Генерация исходного кода
add_custom_target(run_verinc ALL
	COMMAND ${CMAKE_SOURCE_DIR}/verinc  -M ${TRITONN_VER_MAJOR} -m ${TRITONN_VER_MINOR} -p 0 -i -F ${CMAKE_CURRENT_SOURCE_DIR}/build.incver -a \"\" -P TRITONN > ${CMAKE_CURRENT_SOURCE_DIR}/tritonn_version.h
	BYPRODUCTS ${CMAKE_CURRENT_SOURCE_DIR}/tritonn_version.h & sleep 0.5
	COMMENT "Generating build version")
add_dependencies(${PROJECT_NAME} run_verinc)

