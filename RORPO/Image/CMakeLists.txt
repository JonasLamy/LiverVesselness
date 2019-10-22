project(Image)
cmake_minimum_required(VERSION 2.8)

add_subdirectory(Nifti)

# ADD FILES
file(GLOB_RECURSE Image_HEADERS include/*.hpp include/*.h)

install( FILES ${Image_HEADERS} DESTINATION include/Image)
