# OpenNI Samples Android makefile.
# MultipleStreamRead
#

LOCAL_PATH:= $(call my-dir)
include $(CLEAR_VARS) 

# set path to source
MY_PREFIX := $(LOCAL_PATH)/

# list all source files
MY_SRC_FILES := \
	$(MY_PREFIX)*.cpp \

# expand the wildcards
MY_SRC_FILE_EXPANDED := $(wildcard $(MY_SRC_FILES))

# make those paths relative to here
LOCAL_SRC_FILES := $(MY_SRC_FILE_EXPANDED:$(LOCAL_PATH)/%=%)

LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../Include

LOCAL_SHARED_LIBRARIES := OpenNI2

LOCAL_MODULE := MultipleStreamRead

include $(BUILD_EXECUTABLE)
