project(${PROJECT_NAME_PREFIX}open62541)

add_library(${PROJECT_NAME} open62541.h open62541.c)

target_link_libraries(${PROJECT_NAME} PRIVATE crypto ssl pthread)

