

compile_submodule("simpleargs" "" "" "" "")
compile_proto("proto")
compile_submodule("proto" "${protos_out_all}" "" "" "${PROTOBUF_LIBRARY}")

add_subdirectory(shared)
add_subdirectory(xml)
add_subdirectory(open62541)
add_subdirectory(tritonn)

if (NOT DEFINED CMAKE_TRITONN_YOCTO)
	add_subdirectory(ttce)
	add_subdirectory(ttterm)
	add_subdirectory(testevent)
	add_subdirectory(checkxml)
	add_subdirectory(ttutil)

	if(TRITONN_TESTS)
		add_subdirectory(tests)
	endif()
endif()
