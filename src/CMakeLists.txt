
add_subdirectory(shared)
add_subdirectory(open62541)
add_subdirectory(tritonn)
add_subdirectory(ttce)
add_subdirectory(ttterm)
add_subdirectory(testevent)

compile_submodule("simpleargs" "" "")

if(TRITONN_TESTS)
	add_subdirectory(tests)
endif()
