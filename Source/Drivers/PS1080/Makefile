include ../../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

BIN_DIR = ../../../Bin

INC_DIRS = \
	. \
	Include \
	../../../Include \
	../../../ThirdParty/PSCommon/XnLib/Include \
	../../DepthUtils

SRC_FILES = \
	Core/*.cpp	\
	DDK/*.cpp 	\
	DriverImpl/*.cpp\
	Formats/*.cpp	\
	Include/*.cpp	\
	Sensor/*.cpp	\


ifeq ("$(OSTYPE)","Darwin")
	INC_DIRS += \
		/usr/local/include \
		/opt/local/include
	LIB_DIRS += \
		/usr/local/lib \
		/opt/local/lib
	LDFLAGS += \
		-framework CoreFoundation \
		-framework IOKit
endif

LIB_NAME = PS1080

LIB_DIRS += ../../../ThirdParty/PSCommon/XnLib/Bin/$(PLATFORM)-$(CFG)
LIB_DIRS += $(BIN_DIR)/$(PLATFORM)-$(CFG)
USED_LIBS = XnLib dl pthread DepthUtils
ifneq ("$(OSTYPE)","Darwin")
        USED_LIBS += rt usb-1.0 udev
else
	USED_LIBS += usb-1.0.0
endif

CFLAGS += -Wall

OUT_DIR := $(OUT_DIR)/OpenNI2/Drivers

include ../../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
