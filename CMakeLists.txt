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

#set(CMAKE_BUILD_TYPE Release)

#Executable Section:

#Antiga ------------------------

add_executable(Antiga src/Filters/antiga.cpp)
target_include_directories(Antiga PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<INSTALL_INTERFACE:include>"
  )
target_link_libraries(Antiga ${ITK_LIBRARIES}  )
set_target_properties(Antiga PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# Our own Frangi's implementation -----------------------
# In the benchmark, Antiga implementation is used.

add_executable(Frangi src/Filters/frangi.cpp)
target_include_directories(Frangi PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Frangi>"
  "$<INSTALL_INTERFACE:include/Frangi>"
  )
target_link_libraries(Frangi ${ITK_LIBRARIES}  )
set_target_properties(Frangi PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# Jerman -----------------------

add_executable(Jerman src/Filters/jerman.cpp)
target_include_directories(Jerman PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Jerman>"
  "$<INSTALL_INTERFACE:include/Jerman>"
  )
target_link_libraries(Jerman ${ITK_LIBRARIES})
set_target_properties(Jerman PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# Jerman blobness -----------------------

add_executable(JermanBlobness src/Filters/jermanBlobness.cpp)
target_include_directories(JermanBlobness PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/JermanBlobness>"
  "$<INSTALL_INTERFACE:include/JermanBlobness>"
  )
target_link_libraries(JermanBlobness ${ITK_LIBRARIES})
set_target_properties(JermanBlobness PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )



# RhuiZhang -----------------------

add_executable(Zhang src/Filters/ruiZhang.cpp)
target_include_directories(Zhang PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/RuiZhang>"
  "$<INSTALL_INTERFACE:include/RuiZhang>"
  )
target_link_libraries(Zhang ${ITK_LIBRARIES})
set_target_properties(Zhang PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )


# Meijering --------------------------

add_executable(Meijering src/Filters/meijering.cpp)
target_include_directories(Meijering PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Meijering>"
  "$<INSTALL_INTERFACE:include/Meijering>"
  )
target_link_libraries(Meijering ${ITK_LIBRARIES} )
set_target_properties(Meijering PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
# utilities --------------------------------
# Sato -----------------------------

add_executable(Sato src/Filters/sato.cpp)
target_include_directories(Sato PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  )
target_link_libraries(Sato ${ITK_LIBRARIES} )
set_target_properties(Sato PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# OOF ------------------------------

add_executable(OOF src/Filters/oof.cxx)
target_include_directories(OOF PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/OOF>"
  "$<INSTALL_INTERFACE:include/OOF>"
  )
target_link_libraries(OOF ${ITK_LIBRARIES} )
set_target_properties(OOF PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# OOF Geometric mean -----------------

add_executable(OOF_GM src/Filters/oof_GM.cxx)
target_include_directories(OOF_GM PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/OOF>"
  "$<INSTALL_INTERFACE:include/OOF>"
  )
target_link_libraries(OOF_GM ${ITK_LIBRARIES} )
set_target_properties(OOF_GM PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# RORPO ----------------------------

add_subdirectory(RORPO)

# rescale Filter

add_executable(Rescale src/Filters/rescale.cpp)
target_include_directories(Rescale PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
)
target_link_libraries(Rescale ${ITK_LIBRARIES} )
set_target_properties(Rescale PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# benchmark ------------------------

add_subdirectory(lib/jsoncpp)

find_package(jsoncpp REQUIRED)
get_target_property(JSON_INC_PATH jsoncpp_static INTERFACE_INCLUDE_DIRECTORIES)
include_directories(${JSON_INC_PATH})

#include(lib/jsoncpp.cmake)
#if(BENCHMARK_VESSEL_USE_LOCAL_JSONCPP)
#	set(JSONCPP_FOUND ${JSONCPP_FOUND} PARENT_SCOPE)
#	set(JSON_INC_PATH ${JSONCPP_INCLUDE_DIRS} PARENT_SCOPE)
#	set(JSONCPP_LIBRARIES ${JSONCPP_LIBRARIES} PARENT_SCOPE)
#	set(JSONCPP_SOURCES ${JSONCPP_SOURCES} PARENT_SCOPE)
#else()
#   find_package(jsoncpp REQUIRED) 
#   get_target_property(JSON_INC_PATH jsoncpp_lib INTERFACE_INCLUDE_DIRECTORIES)
#   include_directories(${JSON_INC_PATH})
#endif()

add_executable(Benchmark src/Benchmark/benchmark.cpp)

target_include_directories(Benchmark PUBLIC
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Benchmark>"
  "$<INSTALL_INTERFACE:include/Benchmark>"
  )

target_link_libraries(Benchmark ${ITK_LIBRARIES} ${JSONCPP_LIBRARIES}   jsoncpp_lib)
set_target_properties(Benchmark PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )

# TEST METRICS -------------------------------------------

add_executable(Test src/Benchmark/test.cpp)
target_include_directories(Test PUBLIC
"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
"$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include/Benchmark>"
  "$<INSTALL_INTERFACE:include>"
  )
target_link_libraries(Test ${ITK_LIBRARIES} )
set_target_properties(Test PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )



# process ircad -------------------

#add_executable(CropImages src/BDDGeneration/crop_ircad_data.cpp)
#target_include_directories(CropImages PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(CropImages ${ITK_LIBRARIES})
#set_target_properties(CropImages PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts" )

# utilities --------------------------------

#add_executable(ChangeLabelDicom src/BDDGeneration/changeLabelDicom.cpp)
#target_include_directories(ChangeLabelDicom PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(ChangeLabelDicom ${ITK_LIBRARIES})
#set_target_properties(ChangeLabelDicom PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/bin" )
  
#add_executable(MakeIso src/BDDGeneration/makeIso.cpp)
#target_include_directories(MakeIso PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(MakeIso ${ITK_LIBRARIES})
#set_target_properties(MakeIso PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/BDDgenerator/ircad" )

  
#add_executable(MakeIsoBullit src/BDDGeneration/makeIsoBullit.cpp)
#target_include_directories(MakeIsoBullit PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(MakeIsoBullit ${ITK_LIBRARIES})
#set_target_properties(MakeIsoBullit PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/BDDgenerator/bullit" )


#add_executable(MakeVascuSynthGT src/BDDGeneration/makeVascuSynthGT.cpp)
#target_include_directories(MakeVascuSynthGT PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(MakeVascuSynthGT ${ITK_LIBRARIES})
#set_target_properties(MakeVascuSynthGT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts" )

#add_executable(LabelVessels src/BDDGeneration/labelVessels.cpp)
#target_include_directories(LabelVessels PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(LabelVessels ${ITK_LIBRARIES})
#set_target_properties(LabelVessels PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/BDDgenerator" )

#add_executable(LabelVesselsBullit src/BDDGeneration/labelVesselsBullit.cpp)
#target_include_directories(LabelVesselsBullit PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(LabelVesselsBullit ${ITK_LIBRARIES})
#set_target_properties(LabelVesselsBullit PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/BDDgenerator/bullit" )



# Bifurcations VascuSynth

#add_executable(MakeVascuSynthBifurcationGT src/BDDGeneration/makeVascuSynthBifurcationGT.cpp)
#target_include_directories(MakeVascuSynthBifurcationGT PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(MakeVascuSynthBifurcationGT ${ITK_LIBRARIES})
#set_target_properties(MakeVascuSynthBifurcationGT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/BDDgenerator/vascuSynth" )

# Bifurcation IRCAD 

#add_executable(MakeIrcadBifurcationGT src/BDDGeneration/makeIrcadBifurcation.cpp)
#target_include_directories(MakeIrcadBifurcationGT PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(MakeIrcadBifurcationGT ${ITK_LIBRARIES})
#set_target_properties(MakeIrcadBifurcationGT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/BDDgenerator/ircad" )

# Bifurcation Bullit 

#add_executable(MakeBullitBifurcationGT src/BDDGeneration/makeBullitBifurcations.cpp)
#target_include_directories(MakeBullitBifurcationGT PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(MakeBullitBifurcationGT ${ITK_LIBRARIES})
#set_target_properties(MakeBullitBifurcationGT PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/BDDgenerator/bullit" )


# Shapes 

#add_executable(MakeVascuSynthSizeLabels src/BDDGeneration/makeVascuSizeLabels.cpp)
#target_include_directories(MakeVascuSynthSizeLabels PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(MakeVascuSynthSizeLabels ${ITK_LIBRARIES})
#set_target_properties(MakeVascuSynthSizeLabels PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts/BDDgenerator/vascuSynth" )

# isoFromUnit

#add_executable(isoFromUnit src/BDDGeneration/isoFromUnit.cpp)
#target_include_directories(isoFromUnit PUBLIC
#  "$<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/include>"
#  "$<INSTALL_INTERFACE:include>"
#  )
#target_link_libraries(isoFromUnit ${ITK_LIBRARIES})
#set_target_properties(isoFromUnit PROPERTIES RUNTIME_OUTPUT_DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/scripts" )
