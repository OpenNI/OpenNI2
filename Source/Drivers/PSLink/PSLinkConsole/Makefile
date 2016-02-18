include ../../../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

BIN_DIR = ../../../../Bin

INC_DIRS = \
	../../../../Include \
	../../../../ThirdParty/PSCommon/XnLib/Include \
	../ \
	../Protocols/XnLinkProto \
	../LinkProtoLib \

SRC_FILES = \
	*.cpp \
	../LinkProtoLib/*.cpp \
	../*.cpp \

LIB_DIRS = ../../../../ThirdParty/PSCommon/XnLib/Bin/$(PLATFORM)-$(CFG)
USED_LIBS = OpenNI2 XnLib dl pthread

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

ifneq ("$(OSTYPE)","Darwin")
	USED_LIBS += rt usb-1.0 udev
else
	USED_LIBS += usb-1.0.0
endif

CFLAGS += -Wall

EXE_NAME = PSLinkConsole

include ../../../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
