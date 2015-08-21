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
    "include/platform.hpp"
)

# Find required libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
# TODO: add find_package for SDL2 and use result variables.
include_directories( ${OPENGL_INCLUDE_DIRS} ${GLM_INCLUDE_DIR} "${CMAKE_SOURCE_DIR}/include" )
set( COMMON_LIBRARIES "${OPENGL_LIBRARIES}" )
set( PC_LIBRARIES "glew32.a;${OPENGL_LIBRARIES}" )

# Library generation
set( LIBRARY_DST_DIR "${CMAKE_SOURCE_DIR}/../UnityProject/Assets/Plugins" )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY "${LIBRARY_DST_DIR}" )
add_library( ${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries( ${PROJECT_NAME} ${COMMON_LIBRARIES} ${PC_LIBRARIES} )
if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set_target_properties( ${PROJECT_NAME} PROPERTIES
		PREFIX ""
		SUFFIX ".bundle" )
endif()
