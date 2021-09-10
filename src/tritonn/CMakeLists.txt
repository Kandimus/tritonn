project(${PROJECT_NAME_PREFIX}tritonn)

# Sources
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp
							${PROJECT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE files_h ${PROJECT_SOURCE_DIR}/*.h
						  ${CMAKE_BINARY_DIR}/src/*.h)

add_executable(${PROJECT_NAME} ${files_cpp} ${files_h})

target_include_directories(${PROJECT_NAME} PRIVATE ${OPENSSL_INCLUDE_DIRS})
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/candrv")

# OUTPUTNAME
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "tritonn")

# LIBRARIES
add_lib(proto)
add_lib(shared)
add_lib(xml)
add_lib(simpleargs)
add_lib(open62541)

target_link_libraries(${PROJECT_NAME} PRIVATE crypto ssl pthread ${PROTOBUF_LIBRARY})

# YOCTO build
if (DEFINED CMAKE_TRITONN_YOCTO)
	target_link_libraries(${PROJECT_NAME} PRIVATE "${CMAKE_SOURCE_DIR}/redist/candrv/rpmsg_connector.a")
endif()

# INSTALL
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_SOURCE_DIR}/bin/modbus.xml
			  ${CMAKE_SOURCE_DIR}/bin/systemevent.xml
			  ${CMAKE_SOURCE_DIR}/bin/systemtext.xml
			  ${CMAKE_SOURCE_DIR}/bin/tritonn.conf
		DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/cert DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/conf DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/diag DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/ftp  DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/log  DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/www      DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/help DESTINATION ${CMAKE_INSTALL_PREFIX})
install(DIRECTORY ${CMAKE_SOURCE_DIR}/bin/dump DESTINATION ${CMAKE_INSTALL_PREFIX})
