cmake_minimum_required(VERSION 2.6.4)

project(docopt)
include_directories("${PROJECT_SOURCE_DIR}")

########################################################################
# docopt

set(DOCOPT_SRC
	docopt.cpp
	docopt.h
	docopt_private.h
	docopt_util.h
	docopt_value.h
)

add_library(docopt ${LIB_TYPE} ${DOCOPT_SRC})

########################################################################
# installation

INSTALL(TARGETS
	docopt
	DESTINATION lib)

INSTALL(FILES
	docopt.h
	docopt_private.h
	docopt_util.h
	docopt_value.h
	DESTINATION include/docopt)
	
SET(CPACK_PACKAGE_NAME "docopt")
SET(CPACK_DEBIAN_PACKAGE_DEPENDS "")
SET(CPACK_RPM_PACKAGE_REQUIRES "")
SET(CPACK_PACKAGE_DESCRIPTION_SUMMARY "Beautiful command line interfaces")
SET(CPACK_PACKAGE_VENDOR "Jared Grubb")
SET(CPACK_PACKAGE_CONTACT ${CPACK_PACKAGE_VENDOR})
SET(CPACK_PACKAGE_DESCRIPTION_FILE "${PROJECT_SOURCE_DIR}/README.rst")
SET(CPACK_RESOURCE_FILE_LICENSE "${PROJECT_SOURCE_DIR}/LICENSE-MIT")
SET(CPACK_PACKAGE_VERSION_MAJOR 0)
SET(CPACK_PACKAGE_VERSION_MINOR 6)
SET(CPACK_PACKAGE_VERSION_PATCH 1)
SET(CPACK_DEBIAN_PACKAGE_SECTION "Development")
SET(CPACK_RPM_PACKAGE_GROUP "Development/Libraries")
SET(CPACK_RPM_PACKAGE_LICENSE "MIT")
SET(CPACK_STRIP_FILES TRUE)
INCLUDE(CPack)
