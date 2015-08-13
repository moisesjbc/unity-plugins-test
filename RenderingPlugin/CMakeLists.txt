cmake_minimum_required( VERSION 2.8 )
set( CMAKE_BUILD_TYPE Release )

set( ANDROID 1 )

# Compiler flags
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11" )

# Project info
project( RenderingPlugin )
set( PROJECT_VERSION_MAJOR 0 )
set( PROJECT_VERSION_MINOR 1 )
set( PROJECT_VERSION_PATCH 0 )
set( PROJECT_VERSION_STRING ${PROJECT_VERSION_MAJOR}.${PROJECT_VERSION_MINOR}.${PROJECT_VERSION_PATCH} )

# Source files
set( SOURCE_FILES
    "src/RenderingPlugin.cpp"
    "src/lod_plane.cpp"
)

# Header files
set( HEADER_FILES
    "include/RenderingPlugin.h"
    "include/lod_plane.hpp"
    "include/easylogging++.h"
)

# Find required libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
# TODO: add find_package for SDL2 and use result variables.
include_directories( ${OPENGL_INCLUDE_DIRS} ${GLM_INCLUDE_DIR} "${CMAKE_SOURCE_DIR}/include" )
set( LIBRARIES "${OPENGL_LIBRARIES}" )
#message( "ANDROID: [${ANDROID}]" )
if( NOT DEFINED ANDROID )
    set( LIBRARIES "glew32.a;${OPENGL_LIBRARIES}" )
else()
    message( "Android" )
endif()

# Library generation
set( LIBRARY_DST_DIR "${CMAKE_SOURCE_DIR}/../UnityProject/Assets/Plugins" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_DST_DIR}" )
add_library( ${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries( ${PROJECT_NAME} ${LIBRARIES} )
if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set_target_properties( ${PROJECT_NAME} PROPERTIES
		PREFIX ""
		SUFFIX ".bundle" )
endif()

if( DEFINED ANDROID )
    message( "Building Android lib" )
endif()

# Test program generation
if( BUILD_TESTS )
    set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_SOURCE_DIR} )
    add_executable( "tests" "src/tests.cpp" )
    add_dependencies( "tests" ${PROJECT_NAME} )
    target_link_libraries( "tests" "${LIBRARIES};${CMAKE_LIBRARY_OUTPUT_DIRECTORY}/${PROJECT_NAME}.bundle;-lSDL2" )
endif( BUILD_TESTS )
