cmake_minimum_required(VERSION 2.8.12)

project(pyRORPO)

# FIND OPENMP
find_package(OpenMP REQUIRED)
if(OPENMP_FOUND)
    message("OpenMP FOUND")
    set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} ${OpenMP_C_FLAGS}")
    set(CMAKE_CXX_FLAGS "${CMAKE_CXX_FLAGS} ${OpenMP_CXX_FLAGS}")
endif()
    set(CMAKE_EXE_LINKER_FLAGS "${CMAKE_EXE_LINKER_FLAGS} ${OpenMP_EXE_LINKER_FLAGS}")

# Find ITK
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

add_subdirectory("${PROJECT_SOURCE_DIR}/pybind11")

pybind11_add_module(${PROJECT_NAME}
    pyRORPO.cpp
)

# LINK REQUIRED LIBS
target_link_libraries( ${PROJECT_NAME} PRIVATE RORPO ${ITK_LIBRARIES} )