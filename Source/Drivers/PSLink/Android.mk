
LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS) 

include $(LOCAL_PATH)/../../../ThirdParty/PSCommon/BuildSystem/CommonAndroid.mk

# set path to source
MY_PREFIX := $(LOCAL_PATH)

# list all source files
MY_SRC_FILES := \
	$(MY_PREFIX)/*.cpp \
	$(MY_PREFIX)/LinkProtoLib/*.cpp \
	$(MY_PREFIX)/DriverImpl/*.cpp

# expand the wildcards
MY_SRC_FILE_EXPANDED := $(wildcard $(MY_SRC_FILES))

# make those paths relative to here
LOCAL_SRC_FILES := $(MY_SRC_FILE_EXPANDED:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../../Include \
	$(LOCAL_PATH) \
	$(LOCAL_PATH)/Protocols/XnLinkProto \
	$(LOCAL_PATH)/LinkProtoLib \

LOCAL_STATIC_LIBRARIES := XnLib
LOCAL_SHARED_LIBRARIES := libusb

LOCAL_MODULE := PSLink

include $(BUILD_SHARED_LIBRARY)

#include XnLib
include $(LOCAL_PATH)/../../../ThirdParty/PSCommon/XnLib/Source/Android.mk
