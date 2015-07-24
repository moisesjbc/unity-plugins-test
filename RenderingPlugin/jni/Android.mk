LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := RenderingPlugin
LOCAL_SRC_FILES := ../src/lod_plane.cpp ../src/RenderingPlugin.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/.../include

include $(BUILD_SHARED_LIBRARY)
