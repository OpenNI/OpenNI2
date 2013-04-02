# XnCore Android makefile.
# libXnCore.so
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS) 

# set path to source
MY_PREFIX := $(LOCAL_PATH)

# list all source files
MY_SRC_FILES := \
	$(MY_PREFIX)/*.cpp

# expand the wildcards
MY_SRC_FILE_EXPANDED := $(wildcard $(MY_SRC_FILES))

# make those paths relative to here
LOCAL_SRC_FILES := $(MY_SRC_FILE_EXPANDED:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../../../Include \
	$(LOCAL_PATH)/../../../../ThirdParty/PSCommon/XnLib/Include \
	$(LOCAL_PATH)/Include \
	$(LOCAL_PATH)/Protocols/XnLinkProto \

LOCAL_CFLAGS:= -fvisibility=hidden -DXN_CORE_EXPORTS

LOCAL_LDFLAGS += -Wl,--export-dynamic 

LOCAL_MODULE := LinkProtoLib 

include $(BUILD_STATIC_LIBRARY)
