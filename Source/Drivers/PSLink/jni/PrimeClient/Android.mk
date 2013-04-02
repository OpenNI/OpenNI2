# XnCore Android makefile.
# libXnCore.so
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS) 

# set path to source
MY_PREFIX := $(LOCAL_PATH)/../../PrimeClient


# list all source files
MY_SRC_FILES := \
	$(MY_PREFIX)/*.cpp

# expand the wildcards
MY_SRC_FILE_EXPANDED := $(wildcard $(MY_SRC_FILES))

# make those paths relative to here
LOCAL_SRC_FILES := $(MY_SRC_FILE_EXPANDED:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../Include/ \
	$(LOCAL_PATH)/../../LinkProtoLib/Protocols/XnLinkProto \
	$(LOCAL_PATH)/../../LinkProtoLib/Protocols/XnPabloProto \
	$(LOCAL_PATH)/../../LinkProtoLib/Include \
	$(LOCAL_PATH)/../../../../Source/ \

LOCAL_CFLAGS:= -fvisibility=hidden -DXN_CORE_EXPORTS

LOCAL_LDFLAGS += -Wl,--export-dynamic 

LOCAL_SHARED_LIBRARIES := OpenNI
LOCAL_STATIC_LIBRARIES := LinkProtoLib 

LOCAL_PREBUILT_LIBS := libc 

LOCAL_MODULE := PrimeClient

include $(BUILD_SHARED_LIBRARY)
