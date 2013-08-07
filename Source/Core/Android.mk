# OpenNI 2.x Android makefile. 
# Copyright (C) 2012 PrimeSense Ltd.
#
# Licensed under the Apache License, Version 2.0 (the "License");
# you may not use this file except in compliance with the License.
# You may obtain a copy of the License at
#
#      http://www.apache.org/licenses/LICENSE-2.0
#
# Unless required by applicable law or agreed to in writing, software
# distributed under the License is distributed on an "AS IS" BASIS,
# WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
# See the License for the specific language governing permissions and
# limitations under the License. 

LOCAL_PATH := $(call my-dir)

include $(CLEAR_VARS)

# Sources
MY_SRC_FILES := \
	$(LOCAL_PATH)/*.cpp \
	$(LOCAL_PATH)/../Drivers/OniFile/Formats/XnCodec.cpp \
	$(LOCAL_PATH)/../Drivers/OniFile/Formats/XnStreamCompression.cpp
	
ifdef OPENNI2_ANDROID_NDK_BUILD
    MY_SRC_FILES += $(LOCAL_PATH)/../../ThirdParty/LibJPEG/*.c \	
endif
	
MY_SRC_FILE_EXPANDED := $(wildcard $(MY_SRC_FILES))
LOCAL_SRC_FILES := $(MY_SRC_FILE_EXPANDED:$(LOCAL_PATH)/%=%)

# C/CPP Flags
LOCAL_CFLAGS += $(OPENNI2_CFLAGS) -DOPENNI2_EXPORT

# Includes
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../Include \
	$(LOCAL_PATH)/../../ThirdParty/PSCommon/XnLib/Include \
	$(LOCAL_PATH)/../Drivers/OniFile/Formats \
	$(LOCAL_PATH)/../Drivers/OniFile


ifdef OPENNI2_ANDROID_NDK_BUILD
    LOCAL_C_INCLUDES += $(LOCAL_PATH)/../../ThirdParty/LibJPEG
else
    LOCAL_C_INCLUDES += external/jpeg
endif
	
LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../Include

# LD Flags
LOCAL_LDFLAGS := -Wl,--export-dynamic

# Dependencies
LOCAL_STATIC_LIBRARIES := XnLib
LOCAL_SHARED_LIBRARIES := liblog libdl

ifdef OPENNI2_ANDROID_OS_BUILD
    LOCAL_SHARED_LIBRARIES += libjpeg
    LOCAL_REQUIRED_MODULES = libPS1080 libOniFile
else
	LOCAL_LDLIBS += -llog
endif

# Output
LOCAL_MODULE := libOpenNI2

include $(BUILD_SHARED_LIBRARY)
