# RORPO_multiscale usage executable

project (RORPO_multiscale_usage)
cmake_minimum_required(VERSION 3.2)

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

add_executable( ${PROJECT_NAME} main.cpp )

# LINK REQUIRED LIBS
target_link_libraries( ${PROJECT_NAME} RORPO docopt ${ITK_LIBRARIES} )

install( TARGETS ${PROJECT_NAME} RUNTIME DESTINATION bin/ )
