
add_subdirectory(shared)
add_subdirectory(open62541)
add_subdirectory(tritonn)

if(TRITONN_TESTS)
	add_subdirectory(tests)
endif()
