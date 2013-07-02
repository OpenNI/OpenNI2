include ../../../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

BIN_DIR = ../../../../Bin

INC_DIRS = \
	../../../../Include \
	../../../../ThirdParty/PSCommon/XnLib/Include \
	../Include

SRC_FILES = \
	*.cpp \

LIB_DIRS = ../../../../ThirdParty/PSCommon/XnLib/Bin/$(PLATFORM)-$(CFG)
USED_LIBS = XnLib OpenNI2 dl pthread

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

EXE_NAME = PS1080Console

include ../../../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
