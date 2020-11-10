project(${PROJECT_NAME_PREFIX}shared)

# Sources
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp)
file(GLOB_RECURSE files_c ${PROJECT_SOURCE_DIR}/*.c)
file(GLOB_RECURSE files_h ${PROJECT_SOURCE_DIR}/*.h)

add_library(${PROJECT_NAME} STATIC ${files_cpp} ${files_c} ${files_h})

target_link_libraries(${PROJECT_NAME} PRIVATE crypto ssl pthread)

