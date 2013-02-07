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


# Check if we're building from OS or NDK
ifdef TARGET_BUILD_VARIANT
	OPENNI2_ANDROID_OS_BUILD := true
else
	OPENNI2_ANDROID_NDK_BUILD := true
endif

# Setup OpenNI2 local variables
OPENNI2_CFLAGS := -O3 -ftree-vectorize -ffast-math -funroll-loops -fPIC -fvisibility=hidden

ifeq ($(ARCH_ARM_HAVE_ARMV7A),true) 
	OPENNI2_CFLAGS += -march=armv7-a -mfloat-abi=softfp -mtune=cortex-a9 -mfpu=vfp
endif

ifeq ($(ARCH_ARM_HAVE_NEON),true)
	OPENNI2_CFLAGS += -mfpu=neon -DHAVE_NEON=1 -flax-vector-conversions
endif

# Recurse through all subdirs
include $(call all-subdir-makefiles)

# Cleanup the local variables
OPENNI2_CFLAGS := 
