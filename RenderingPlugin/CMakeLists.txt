cmake_minimum_required( VERSION 2.8 )
set( CMAKE_BUILD_TYPE Release )

# Compiler flags
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11" )

# Project info
project( RenderingPlugin )
set( PROJECT_VERSION_MAJOR 0 )
set( PROJECT_VERSION_MINOR 1 )
set( PROJECT_VERSION_PATCH 0 )
set( PROJECT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} )

# Find required libraries
find_package(OpenGL REQUIRED)
find_package(GLUT REQUIRED)
# TODO: add find_package for SDL2 and use result variables.
include_directories( ${OPENGL_INCLUDE_DIRS}  ${GLUT_INCLUDE_DIRS} "${CMAKE_SOURCE_DIR}/include" )
set( LIBRARIES ${OPENGL_LIBRARIES} ${GLUT_LIBRARY} )

# Library generation
set( LIBRARY_DST_DIR "${CMAKE_SOURCE_DIR}/../UnityProject/Assets/Plugins" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_DST_DIR}" )
add_library( ${PROJECT_NAME} SHARED "src/RenderingPlugin.cpp" "include/RenderingPlugin.h" )
set_target_properties( ${PROJECT_NAME} PROPERTIES
	PREFIX ""
	SUFFIX ".bundle"
)
target_link_libraries( ${PROJECT_NAME} ${LIBRARIES} )

# Test program generation
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
add_executable( "tests" "src/tests.cpp" )
add_dependencies( "tests" ${PROJECT_NAME} )
target_link_libraries( "tests" "${LIBRARIES};${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.bundle;-lSDL2" )