cmake_minimum_required( VERSION 2.8 )
set( CMAKE_BUILD_TYPE Release )

set( ANDROID 1 )

# Compiler flags
set( CMAKE_CXX_FLAGS  "${CMAKE_CXX_FLAGS} -std=c++11" )
add_definitions( "-std=c++11" )

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
    "src/shaders.cpp"
)

# Header files
set( HEADER_FILES
    "include/RenderingPlugin.h"
    "include/lod_plane.hpp"
    "include/easylogging++.h"
    "include/platform.hpp"
    "include/shaders.hpp"
)

# Find required libraries
set(CMAKE_MODULE_PATH ${CMAKE_MODULE_PATH} "${CMAKE_SOURCE_DIR}/cmake/Modules/")
find_package(OpenGL REQUIRED)
find_package(GLM REQUIRED)
# TODO: add find_package for SDL2 and use result variables.
include_directories( ${OPENGL_INCLUDE_DIRS} ${GLM_INCLUDE_DIR} "${CMAKE_SOURCE_DIR}/include" )
set( COMMON_LIBRARIES "${OPENGL_LIBRARIES}" )
set( PC_LIBRARIES "glew32;${OPENGL_LIBRARIES};" )

# Output directory
set( UNITY_PLUGINS_DIR "${CMAKE_BINARY_DIR}/../UnityProject/Assets/Plugins" )
set( CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${UNITY_PLUGINS_DIR} )
set( CMAKE_LIBRARY_OUTPUT_DIRECTORY ${UNITY_PLUGINS_DIR} )
set( CMAKE_RUNTIME_OUTPUT_DIRECTORY ${UNITY_PLUGINS_DIR} )

# Library generation
add_library( ${PROJECT_NAME} SHARED ${SOURCE_FILES} ${HEADER_FILES} )
target_link_libraries( ${PROJECT_NAME} ${PC_LIBRARIES} )
if( ${CMAKE_SYSTEM_NAME} MATCHES "Darwin")
	set_target_properties( ${PROJECT_NAME} PROPERTIES
		SUFFIX ".bundle" )
endif()
set_target_properties( ${PROJECT_NAME} PROPERTIES PREFIX "" )

# Configure Android build
set( ANDROID_SOURCE_FILES "" )

foreach( SOURCE_FILE ${SOURCE_FILES} )
    set( ANDROID_SOURCE_FILES "${ANDROID_SOURCE_FILES} ../${SOURCE_FILE}" )
endforeach( SOURCE_FILE SOURCE_FILES )

set( UNITY_ANDROID_PLUGINS_DIR "${UNITY_PLUGINS_DIR}/Android" )

configure_file( "jni/Android.mk.in" "jni/Android.mk" )
configure_file( "jni/Application.mk.in" "jni/Application.mk" )

add_custom_target( ${PROJECT_NAME}_android ALL
    "ndk-build" "-B"
    COMMAND ${CMAKE_COMMAND} -E make_directory ${UNITY_ANDROID_PLUGINS_DIR}
    COMMAND ${CMAKE_COMMAND} -E copy ${CMAKE_BINARY_DIR}/libs/armeabi-v7a/libRenderingPlugin.so ${UNITY_ANDROID_PLUGINS_DIR}
)
