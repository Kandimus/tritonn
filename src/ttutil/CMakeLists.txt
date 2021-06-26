project(${PROJECT_NAME_PREFIX}ttutil)

# Sources
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp
							${PROJECT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE files_h	${PROJECT_SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME} ${files_cpp} ${files_h})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "ttutil")

add_lib(shared)
add_lib(simpleargs)
add_lib(xml)

