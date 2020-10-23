project(${PROJECT_NAME_PREFIX}tests)

# Sources
file(GLOB_RECURSE files_cpp ${PROJECT_SOURCE_DIR}/*.cpp
									 ${CMAKE_SOURCE_DIR}/src/tritonn/*.cpp
									 ${CMAKE_SOURCE_DIR}/src/tritonn/*.c)
file(GLOB_RECURSE files_h   ${CMAKE_SOURCE_DIR}/src/tritonn/*.h)

add_executable(${PROJECT_NAME} ${files_cpp} ${files_h})

install(TARGETS ${PROJECT_NAME} DESTINATION ${CMAKE_INSTALL_PREFIX})
install(FILES ${CMAKE_SOURCE_DIR}/bin/conf/test.xml DESTINATION ${CMAKE_INSTALL_PREFIX}/conf)

set_target_properties(${PROJECT_NAME} PROPERTIES OUTPUT_NAME "test")

add_lib(shared)
add_lib(open62541)
add_lib(simpleargs)
add_lib(simpletest)

target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/tritonn")
target_include_directories(${PROJECT_NAME} PUBLIC "${CMAKE_SOURCE_DIR}/src/tests")

target_compile_definitions(${PROJECT_NAME} PRIVATE TRITONN_TEST=1)

find_package( PythonInterp 3.0 REQUIRED )

# Генерация исходного кода
add_custom_target(generating_cpp ALL
	COMMAND python3 ${CMAKE_SOURCE_DIR}/src/simpletest/simpletest.py --output ${CMAKE_SOURCE_DIR}/src/tests/simpletest.gen.cpp --dir ${CMAKE_SOURCE_DIR}/src/tests --files test_ai.cpp
	BYPRODUCTS ${CMAKE_SOURCE_DIR}/src/tests/simpletest.gen.cpp & sleep 0.5
	COMMENT "Generating simple test file")
add_dependencies(${PROJECT_NAME} generating_cpp)


list(APPEND ${SIMPLETEST_ADDDIR} "${CMAKE_SOURCE_DIR}/src/simpleargs")
if(SIMPLETEST_USING_SIMPLEARGS)
	list(APPEND ${SIMPLETEST_ADDDEF} "SIMPLETEST_USING_SIMPLEARGS=1")
endif()

compile_submodule("simpletest" SIMPLETEST_ADDDIR SIMPLETEST_ADDDEF)
