# create .bin file

if(CMAKE_TRITONN_YOCTO)
	set(CPACK_PACKAGE_VERSION "${PROJECT_VERSION_STRING}")
	set(CPACK_TRITONN_FILENAME "${PROJECT_NAME}_${CPACK_PACKAGE_VERSION}")
	set(CPACK_GENERATOR "ZIP")
	set(CPACK_SET_DESTDIR ON)
	set(CPACK_PACKAGE_FILE_NAME "${CPACK_TRITONN_FILENAME}")
	set(CPACK_INCLUDE_TOPLEVEL_DIRECTORY OFF)

	message(STATUS "Create package: ${CPACK_TRITONN_FILENAME}.zip")

	include(CPack)

	add_custom_target(package_ssl COMMAND
		openssl enc -e -aes256 -pbkdf2 -k Kx48dHnZ@o3nE8L%pfNu3V9rM8g@n7 -in ${CPACK_TRITONN_FILENAME}.zip -out ${CPACK_TRITONN_FILENAME}.bin
		DEPENDS package
	)
	add_dependencies(package_ssl package)
endif()
