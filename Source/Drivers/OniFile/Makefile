include ../../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

BIN_DIR = ../../../Bin

INC_DIRS = \
	. \
	../../../Include \
	../../../ThirdParty/PSCommon/XnLib/Include \
	Formats

SRC_FILES = \
	*.cpp \
	Formats/*.cpp \
	XnLibExtensions/*.cpp \

ifeq ("$(OSTYPE)","Darwin")
	INC_DIRS += \
		/usr/local/include \
		/opt/local/include
	LIB_DIRS += \
		/usr/local/lib \
		/opt/local/lib
endif

LIB_NAME = OniFile

LIB_DIRS = ../../../ThirdParty/PSCommon/XnLib/Bin/$(PLATFORM)-$(CFG)
USED_LIBS = XnLib pthread dl
ifneq ("$(OSTYPE)","Darwin")
        USED_LIBS += rt  
endif

CFLAGS += -Wall

OUT_DIR := $(OUT_DIR)/OpenNI2/Drivers

include ../../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
