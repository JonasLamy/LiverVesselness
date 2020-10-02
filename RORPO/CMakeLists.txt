project(RORPO)
cmake_minimum_required(VERSION 3.2)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

if(UNIX)
	SET(LIB_TYPE SHARED)
else()
	SET(LIB_TYPE STATIC)
endif()

# Build Type
if(NOT CMAKE_BUILD_TYPE)
   SET(CMAKE_BUILD_TYPE Release
       CACHE STRING "Choose the type of build : Debug or Release."
       FORCE)
endif()
message("Build Type: " ${CMAKE_BUILD_TYPE})

include_directories(
            libRORPO/include
            Image/include
            docopt)

add_subdirectory(libRORPO)

add_subdirectory(RORPO_multiscale_Usage)
