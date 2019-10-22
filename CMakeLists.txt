cmake_minimum_required(VERSION 3.9.5)
 
project(Vesselness)
 

find_package(ITK 5.1 REQUIRED)
include(${ITK_USE_FILE})

FIND_PACKAGE(Boost 1.46.0 REQUIRED COMPONENTS program_options)
if ( Boost_FOUND )
  message(STATUS "Boost and boost_program_options found.")
  ADD_DEFINITIONS(${BOOST_DEFINITIONS} -DBOOST_ALL_NO_LIB)    
  include_directories( ${Boost_INCLUDE_DIRS} )
  SET(Boost_OPTIONS ${Boost_LIBRAIRIES} ${Boost_PROGRAM_OPTIONS_LIBRARY})
endif( Boost_FOUND )

include_directories(lib)

add_subdirectory(Sato)
add_subdirectory(Antiga)
add_subdirectory(Zhang)
add_subdirectory(Jerman)
add_subdirectory(Meijering)
add_subdirectory(RuiZhang)
add_subdirectory(OOF)
add_subdirectory(RORPO)

add_executable(checkGaussian checkGaussian.cpp)
target_link_libraries(checkGaussian ${Boost_OPTIONS} ${ITK_LIBRARIES})
