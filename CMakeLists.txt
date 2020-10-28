cmake_minimum_required(VERSION 3.9.5)
 
project(Vesselness)

set(CMAKE_CXX_STANDARD 11)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

if(APPLE)
    set(CMAKE_OSX_SYSROOT /Library/Developer/CommandLineTools/SDKs/MacOSX.sdk)
endif()



# CLI11
include_directories( "${PROJECT_SOURCE_DIR}/lib/" )



find_package(ITK 5.2 REQUIRED)
include(${ITK_USE_FILE})

set(CMAKE_BUILD_TYPE Release)

#Executable Section:

#Antiga ------------------------

add_executable(Antiga src/antiga.cpp)
target_include_directories(Antiga PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
  )
target_link_libraries(Antiga ${ITK_LIBRARIES}  )
set_target_properties(Antiga PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# Jerman -----------------------

add_executable(Jerman src/jerman.cpp)
target_include_directories(Jerman PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Jerman>"
  "$<INSTALL_INTERFACE:include/Jerman>"
  )
target_link_libraries(Jerman ${ITK_LIBRARIES})
set_target_properties(Jerman PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# RhuiZhang -----------------------

add_executable(RuiZhang src/ruiZhang.cpp)
target_include_directories(RuiZhang PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/RuiZhang>"
  "$<INSTALL_INTERFACE:include/RuiZhang>"
  )
target_link_libraries(RuiZhang ${ITK_LIBRARIES})
set_target_properties(RuiZhang PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )


# Meijering --------------------------

add_executable(Meijering src/meijering.cpp)
target_include_directories(Meijering PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Meijering>"
  "$<INSTALL_INTERFACE:include/Meijering>"
  )
target_link_libraries(Meijering ${ITK_LIBRARIES} )
set_target_properties(Meijering PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# Sato -----------------------------

add_executable(Sato src/sato.cpp)
target_include_directories(Sato PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  )
target_link_libraries(Sato ${ITK_LIBRARIES} )
set_target_properties(Sato PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# OOF ------------------------------

add_executable(OOF src/oof.cxx)
target_include_directories(OOF PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/OOF>"
  "$<INSTALL_INTERFACE:include/OOF>"
  )
target_link_libraries(OOF ${ITK_LIBRARIES} )
set_target_properties(OOF PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# RORPO ----------------------------

add_subdirectory(RORPO)

# benchmark ------------------------

find_package(jsoncpp REQUIRED)
get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)
include_directories(${JSON_INC_PATH})

add_executable(Benchmark src/benchmark.cpp)
target_include_directories(Benchmark PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Benchmark>"
  "$<INSTALL_INTERFACE:include/Benchmark>"
  )
target_link_libraries(Benchmark ${ITK_LIBRARIES}    jsoncpp_lib)
set_target_properties(Benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# process ircad -------------------

#add_executable(CropImages src/crop_ircad_data.cpp)
#target_include_directories(CropImages PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(CropImages ${ITK_LIBRARIES})
#set_target_properties(CropImages PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts" )

# utilities --------------------------------

#add_executable(ChangeLabelDicom src/changeLabelDicom.cpp)
#target_include_directories(ChangeLabelDicom PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(ChangeLabelDicom ${ITK_LIBRARIES})
#set_target_properties(ChangeLabelDicom PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

#add_executable(MakeIso src/makeIso.cpp)
#target_include_directories(MakeIso PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(MakeIso ${ITK_LIBRARIES})
#set_target_properties(MakeIso PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts" )

#add_executable(MakeVascuSynthGT src/makeVascuSynthGT.cpp)
#target_include_directories(MakeVascuSynthGT PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(MakeVascuSynthGT ${ITK_LIBRARIES})
#set_target_properties(MakeVascuSynthGT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts" )

# Bifurcations VascuSynth

#add_executable(MakeVascuSynthBifurcationGT src/makeVascuSynthBifurcationGT.cpp)
#target_include_directories(MakeVascuSynthBifurcationGT PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(MakeVascuSynthBifurcationGT ${ITK_LIBRARIES})
#set_target_properties(MakeVascuSynthBifurcationGT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts" )

# Bifurcation IRCAD 

#add_executable(MakeIrcadBifurcationGT src/makeIrcadBifurcation.cpp)
#target_include_directories(MakeIrcadBifurcationGT PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(MakeIrcadBifurcationGT ${ITK_LIBRARIES})
#set_target_properties(MakeIrcadBifurcationGT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts" )

# isoFromUnit local script

#add_executable(isoFromUnit src/isoFromUnit.cpp)
#target_include_directories(isoFromUnit PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(isoFromUnit ${ITK_LIBRARIES})
#set_target_properties(isoFromUnit PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts" )

# TEST METRICS

#add_executable(Test src/test.cpp)
#target_include_directories(Test PUBLIC
#"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Benchmark>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(Test ${ITK_LIBRARIES} )
#set_target_properties(Test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

