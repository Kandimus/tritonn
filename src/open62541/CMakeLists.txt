
add_library(open62541 open62541.h open62541.c)

target_link_libraries(open62541 PRIVATE crypto ssl)
target_link_libraries(open62541 PRIVATE pthread)

#target_include_directories(open62541 ".")
