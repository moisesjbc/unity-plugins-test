cmake_minimum_required( VERSION 2.8 )
set( CMAKE_BUILD_TYPE Release )

# Project info
project( RenderingPlugin )
set( PROJECT_VERSION_MAJOR 0 )
set( PROJECT_VERSION_MINOR 1 )
set( PROJECT_VERSION_PATCH 0 )
set( PROJECT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} )

# Find required libraries
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} )
set( LIBRARIES ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )

# Library generation
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${CMAKE_SOURCE_DIR}/../UnityProject/Assets/Plugins" )
add_library( ${PROJECT_NAME} SHARED "RenderingPlugin.cpp" "UnityPluginInterface.h" )
set_target_properties( ${PROJECT_NAME} PROPERTIES 
	PREFIX ""
	SUFFIX ".bundle"
)
target_link_libraries( ${PROJECT_NAME} ${LIBRARIES} )
