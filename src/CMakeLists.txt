
compile_submodule("simpleargs" "" "")

add_subdirectory(shared)
add_subdirectory(xml)
add_subdirectory(open62541)
add_subdirectory(tritonn)
add_subdirectory(ttce)
add_subdirectory(ttterm)
add_subdirectory(testevent)
add_subdirectory(checkxml)

if(TRITONN_TESTS)
	add_subdirectory(tests)
endif()
