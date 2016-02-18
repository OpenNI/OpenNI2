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

include $(LOCAL_PATH)/../../ThirdParty/PSCommon/BuildSystem/CommonAndroid.mk

# Sources
MY_SRC_FILES := \
	$(LOCAL_PATH)/*.cpp \
	$(LOCAL_PATH)/../Drivers/OniFile/Formats/XnCodec.cpp \
	$(LOCAL_PATH)/../Drivers/OniFile/Formats/XnStreamCompression.cpp
	
MY_SRC_FILE_EXPANDED := $(wildcard $(MY_SRC_FILES))
LOCAL_SRC_FILES := $(MY_SRC_FILE_EXPANDED:$(LOCAL_PATH)/%=%)

# C/CPP Flags
LOCAL_CFLAGS += -DOPENNI2_EXPORT

# Includes
LOCAL_C_INCLUDES := \
	$(LOCAL_PATH)/../../Include \
	$(LOCAL_PATH)/../Drivers/OniFile/Formats \
	$(LOCAL_PATH)/../Drivers/OniFile

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/../../Include

# LD Flags
LOCAL_LDFLAGS := -Wl,--export-dynamic

# Dependencies
LOCAL_STATIC_LIBRARIES := XnLib

ifdef PS_OS_BUILD
    LOCAL_SHARED_LIBRARIES += libjpeg
    LOCAL_REQUIRED_MODULES = libPS1080 libOniFile
endif

# Output
LOCAL_MODULE := libOpenNI2

include $(BUILD_SHARED_LIBRARY)

#include XnLib
include $(LOCAL_PATH)/../../ThirdParty/PSCommon/XnLib/Source/Android.mk
