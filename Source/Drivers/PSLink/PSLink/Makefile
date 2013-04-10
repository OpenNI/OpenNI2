include ../../../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

BIN_DIR = ../../../../Bin

INC_DIRS = \
	../../../../Include \
	../../../../ThirdParty/PSCommon/XnLib/Include \
	. \
	../LinkProtoLib/Protocols/XnLinkProto \
	../LinkProtoLib/Protocols/XnPabloProto \
	../LinkProtoLib/Include \

SRC_FILES = \
	*.cpp \
	DriverImpl/LinkExportedOniDriver.cpp \
	DriverImpl/LinkOniDriver.cpp \
	DriverImpl/LinkOniDevice.cpp \
	DriverImpl/LinkOniStream.cpp \
	DriverImpl/LinkOniMapStream.cpp \
	DriverImpl/LinkOniIRStream.cpp \
	DriverImpl/LinkOniDepthStream.cpp \
#	DriverImpl/*.cpp \

LIB_DIRS  = ../../../../ThirdParty/PSCommon/XnLib/Bin/$(PLATFORM)-$(CFG)
LIB_DIRS += $(BIN_DIR)/$(PLATFORM)-$(CFG)
USED_LIBS = LinkProtoLib XnLib dl pthread

ifeq ("$(OSTYPE)","Darwin")
	INC_DIRS += /opt/local/include
	LIB_DIRS += /opt/local/lib
	LDFLAGS += -framework CoreFoundation -framework IOKit
endif

ifneq ("$(OSTYPE)","Darwin")
	USED_LIBS += rt usb-1.0 udev
else
	USED_LIBS += usb-1.0.0
endif

CFLAGS += -Wall

LIB_NAME = PSLink

OUT_DIR := $(OUT_DIR)/OpenNI2/Drivers

include ../../../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
