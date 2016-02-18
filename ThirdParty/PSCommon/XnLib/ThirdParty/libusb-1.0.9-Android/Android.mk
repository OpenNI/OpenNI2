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

# *** 
# *** Note: This module will only get built if compiled via the NDK! ***
# *** 

LOCAL_PATH:= $(call my-dir)

include $(CLEAR_VARS)

include $(LOCAL_PATH)/../../../BuildSystem/CommonAndroid.mk

ifdef PS_OS_BUILD
	$(info Skipping libusb in OS build...)
else

ifndef PS_NO_USB

# Sources
LOCAL_SRC_FILES:= \
                  libusb/core.c \
                  libusb/descriptor.c \
                  libusb/io.c \
                  libusb/sync.c \
                  libusb/os/linux_usbfs.c \
                  libusb/os/threads_posix.c

# Includes				  
LOCAL_C_INCLUDES += \
                    $(LOCAL_PATH)/libusb/ \
                    $(LOCAL_PATH)/libusb/os/

# Dependencies
LOCAL_LDLIBS += -llog

# Output
LOCAL_MODULE := libusb

LOCAL_EXPORT_C_INCLUDES := $(LOCAL_PATH)/libusb

include $(BUILD_SHARED_LIBRARY)

endif #NO_USB
endif #OS_BUILD
