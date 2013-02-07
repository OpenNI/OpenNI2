# XnLib Android makefile.
# libXnLib.so
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS) 

# set path to source
MY_PREFIX := $(LOCAL_PATH)/

# list all source files
MY_SRC_FILES := \
	$(MY_PREFIX)*.cpp \
    $(MY_PREFIX)Linux/*.cpp

# expand the wildcards
MY_SRC_FILE_EXPANDED := $(wildcard $(MY_SRC_FILES))

# make those paths relative to here
LOCAL_SRC_FILES := $(MY_SRC_FILE_EXPANDED:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../Include \
	$(LOCAL_PATH)/../ThirdParty/libusb-1.0.9-Android/libusb \

LOCAL_LDFLAGS := -llog

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../../../Include

LOCAL_SHARED_LIBRARIES := libusb 
LOCAL_PREBUILT_LIBS := libc 

LOCAL_MODULE := XnLib

include $(BUILD_STATIC_LIBRARY)


