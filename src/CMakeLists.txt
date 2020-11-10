
add_subdirectory(shared)
add_subdirectory(open62541)
add_subdirectory(tritonn)
add_subdirectory(ttce)

compile_submodule("simpleargs" "" "")

if(TRITONN_TESTS)
	add_subdirectory(tests)
endif()
