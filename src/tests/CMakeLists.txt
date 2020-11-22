project(${PROJECT_NAME_PREFIX}tests)

# Sources
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp
							${CMAKE_SOURCE_DIR}/src/tritonn/*.cpp
							${CMAKE_SOURCE_DIR}/src/tritonn/*.c)
file(GLOB_RECURSE files_h   ${CMAKE_SOURCE_DIR}/src/tritonn/*.h)

add_executable(${PROJECT_NAME} ${files_cpp} ${files_h})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_SOURCE_DIR}/bin/conf/test.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/conf)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "test")

# LIBRARIES
add_lib(shared)
add_lib(open62541)
add_lib(simpleargs)

target_link_libraries(${PROJECT_NAME} PRIVATE crypto ssl pthread)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/tritonn")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/tests")
#target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/src")

target_compile_definitions(${PROJECT_NAME} PRIVATE TRITONN_TEST=1)

