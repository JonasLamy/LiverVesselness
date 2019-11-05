cmake_minimum_required(VERSION 3.9.5)
 
project(benchmark)
 
find_package(ITK REQUIRED)
include(${ITK_USE_FILE})

FIND_PACKAGE(Boost 1.46.0 REQUIRED COMPONENTS program_options)
if ( Boost_FOUND )
  message(STATUS "Boost and boost_program_options found.")
  ADD_DEFINITIONS(${BOOST_DEFINITIONS} -DBOOST_ALL_NO_LIB)    
  include_directories( ${Boost_INCLUDE_DIRS} )
  SET(Boost_OPTIONS ${Boost_LIBRAIRIES} ${Boost_PROGRAM_OPTIONS_LIBRARY})
endif( Boost_FOUND )

FIND_PACKAGE(jsoncpp REQUIRED)
include_directories( ${JSONCPP_INCLUDE_DIR})

add_executable(benchmark benchmark.cpp)
target_link_libraries(benchmark ${Boost_OPTIONS} ${ITK_LIBRARIES} jsoncpp)

file(MAKE_DIRECTORY ./build/output)
