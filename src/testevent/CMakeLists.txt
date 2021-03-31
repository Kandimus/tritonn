project(${PROJECT_NAME_PREFIX}testevent)

# Sources
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp
							${PROJECT_SOURCE_DIR}/*.c
							${PROJECT_SOURCE_DIR}/../tritonn/event/event.cpp)
file(GLOB_RECURSE files_h	${PROJECT_SOURCE_DIR}/*.h)

add_executable(${PROJECT_NAME} ${files_cpp} ${files_h})

# OUTPUTNAME
set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "testevent")

# LIBRARIES
target_link_libraries(${PROJECT_NAME} PRIVATE crypto ssl pthread)

add_lib(shared)
add_lib(simpleargs)

target_include_directories(${PROJECT_NAME} PRIVATE "${CMAKE_BINARY_DIR}/src")

# INSTALL
install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
