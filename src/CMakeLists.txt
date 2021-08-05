

compile_submodule("simpleargs" "" "" "" "")
compile_proto("proto")
compile_submodule("proto" "${protos_out_all}" "" "" "protobuf-lite")

add_subdirectory(shared)
add_subdirectory(xml)
add_subdirectory(open62541)
add_subdirectory(tritonn)
add_subdirectory(ttce)
add_subdirectory(ttterm)
add_subdirectory(testevent)
add_subdirectory(checkxml)
add_subdirectory(ttutil)

if(TRITONN_TESTS)
	add_subdirectory(tests)
endif()
