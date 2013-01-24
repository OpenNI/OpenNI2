# OpenNI OniFile Android makefile.
# libOniFile.so
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS) 

# set path to source
MY_PREFIX := $(LOCAL_PATH)/

# list all source files
MY_SRC_FILES := \
	$(MY_PREFIX)*.cpp \
	$(MY_PREFIX)Formats/*.cpp \
	$(MY_PREFIX)XnLibExtensions/*.cpp \
	$(MY_PREFIX)../../../ThirdParty/LibJPEG/*.c

# expand the wildcards
MY_SRC_FILE_EXPANDED := $(wildcard $(MY_SRC_FILES))

# make those paths relative to here
LOCAL_SRC_FILES := $(MY_SRC_FILE_EXPANDED:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/. \
	$(LOCAL_PATH)/../../../Include \
	$(LOCAL_PATH)/../../../ThirdParty/PSCommon/XnLib/Include \
	$(LOCAL_PATH)/../../../ThirdParty/LibJPEG \
	$(LOCAL_PATH)/Formats

LOCAL_CFLAGS := -fvisibility=hidden
LOCAL_CPPFLAGS := -frtti
LOCAL_LDFLAGS := -Wl,--export-dynamic -llog

LOCAL_PREBUILT_LIBS := libc
LOCAL_STATIC_LIBRARIES := XnLib

LOCAL_MODULE:= OniFile

include $(BUILD_SHARED_LIBRARY)

