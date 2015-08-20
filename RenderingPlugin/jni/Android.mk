LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

LOCAL_MODULE    := RenderingPlugin
LOCAL_SRC_FILES := ../src/lod_plane.cpp ../src/RenderingPlugin.cpp
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../include
LOCAL_CFLAGS := -DUNITY_ANDROID -std=gnu++11 $(LOCAL_CFLAGS)

include $(BUILD_SHARED_LIBRARY)
