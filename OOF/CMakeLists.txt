#Change PROJECT_NAME to the name of your project
PROJECT(Vesselness)

#include some macros from another file...
INCLUDE(IJMacros.txt)

#Declare any external dependencies that your project may have here.
#examples include: ITK, VTK, JPEG, PNG, OpenGL, ZLIB, Perl, Java
#If you're not sure what name to use, look in the Modules directory of your
#cmake install and check that a file named Find(Package).cmake exists
#
# The packages can be specified with a version number, for example:
#
# ITK 2.8.1
# ITK 3.2.0
# 
# If no version is specified, the most recent release of the package
# will be used.
SET(Required_Packages
ITK
)

set(CMAKE_BUILD_TYPE release) #debug none release distribution

#this foreach loads all of the packages that you specified as required.
#It shouldn't need to be modified.
FOREACH(Package ${Required_Packages})
  LOADPACKAGE(${Package})
ENDFOREACH(Package)

#Set any libraries that your project depends on.
#examples: ITKCommon, VTKRendering, etc
SET(Libraries
${ITK_LIBRARIES} ${Boost_OPTIONS}
)
INCLUDE(${ITK_USE_FILE})
include_directories(${PROJECT_SOURCE_DIR})
message( ${PROJECT_SOURCE_DIR} )

SET(CurrentExe "lightMultiScaleTubularityImageFilter")
ADD_EXECUTABLE(lightMultiScaleTubularityImageFilter simplifiedTubularityFilterApplication.cxx)
TARGET_LINK_LIBRARIES(lightMultiScaleTubularityImageFilter ${Libraries})









