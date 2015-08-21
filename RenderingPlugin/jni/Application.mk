APP_ABI := armeabi-v7a
APP_PLATFORM := android-14
APP_STL := stlport_static
#  Enable C++11. However, pthread, rtti and exceptions aren’t enabled
APP_CPPFLAGS += -std=c++11
APP_CPPFLAGS += -fexceptions
# Instruct to use the static GNU STL implementation
APP_STL := gnustl_static
