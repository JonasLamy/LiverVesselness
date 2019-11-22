cmake_minimum_required(VERSION 3.9.5)
 
project(Vesselness)
 
find_package(ITK 5.0 REQUIRED)
include(${ITK_USE_FILE})
find_package(Boost 1.46.0 REQUIRED COMPONENTS program_options)


#Executable Section:

#Antiga ------------------------

add_executable(Antiga src/antiga.cpp)
target_include_directories(Antiga PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
  )
target_link_libraries(Antiga ${ITK_LIBRARIES} Boost::program_options )
set_target_properties(Antiga PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# Jerman -----------------------

add_executable(Jerman src/jerman.cpp)
target_include_directories(Jerman PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Jerman>"
  "$<INSTALL_INTERFACE:include/Jerman>"
  )
target_link_libraries(Jerman ${ITK_LIBRARIES} Boost::program_options)
set_target_properties(Jerman PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# Meijering --------------------------

add_executable(Meijering src/meijering.cpp)
target_include_directories(Meijering PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Meijering>"
  "$<INSTALL_INTERFACE:include/Meijering>"
  )
target_link_libraries(Meijering ${ITK_LIBRARIES} Boost::program_options)
set_target_properties(Meijering PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# Sato -----------------------------

add_executable(Sato src/sato.cpp)
target_include_directories(Sato PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  )
target_link_libraries(Sato ${ITK_LIBRARIES} Boost::program_options)
set_target_properties(Sato PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# OOF ------------------------------

add_executable(OOF src/oof.cxx)
target_include_directories(OOF PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/OOF>"
  "$<INSTALL_INTERFACE:include/OOF>"
  )
target_link_libraries(OOF ${ITK_LIBRARIES} Boost::program_options)
set_target_properties(OOF PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# RORPO ----------------------------

add_subdirectory(RORPO)

# benchmark ------------------------

find_package(jsoncpp REQUIRED)
add_executable(Benchmark src/benchmark.cpp)
target_include_directories(Benchmark PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Benchmark>"
  "$<INSTALL_INTERFACE:include/Benchmark>"
  )
target_link_libraries(Benchmark ${ITK_LIBRARIES} Boost::program_options jsoncpp)
set_target_properties(Benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# process ircad -------------------

add_executable(CropImages src/process_ircad_data.cpp)
target_include_directories(CropImages PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
  )
target_link_libraries(CropImages ${ITK_LIBRARIES})
set_target_properties(CropImages PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../scripts" )

# temp --------------------------------

add_executable(Tot src/tot.cpp)
target_include_directories(Tot PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
  )
target_link_libraries(Tot ${ITK_LIBRARIES})
set_target_properties(Tot PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

add_executable(MakeIso src/makeIso.cpp)
target_include_directories(MakeIso PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
  )
target_link_libraries(MakeIso ${ITK_LIBRARIES})
set_target_properties(MakeIso PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../scripts" )


