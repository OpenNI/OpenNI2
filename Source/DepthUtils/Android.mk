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

MY_SRC_FILE_EXPANDED := $(wildcard $(MY_SRC_FILES))
LOCAL_SRC_FILES := $(MY_SRC_FILE_EXPANDED:$(LOCAL_PATH)/%=%)

# Includes
LOCAL_C_INCLUDES := $(LOCAL_PATH)/../../Include

LOCAL_STATIC_LIBRARIES := XnLib

# Output
LOCAL_MODULE := DepthUtils

include $(BUILD_STATIC_LIBRARY)

#include XnLib
include $(LOCAL_PATH)/../../ThirdParty/PSCommon/XnLib/Source/Android.mk
