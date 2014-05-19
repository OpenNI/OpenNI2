include ../../BuildSystem/CommonDefs.mak

BIN_DIR = ../Bin

INC_DIRS = \
	../Include \
	../ThirdParty/LibJPEG

SRC_FILES = \
	*.cpp \
	Linux/*.cpp\
	../ThirdParty/LibJPEG/*.c

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

SLIB_NAME = XnLib
USED_LIBS = usb-1.0 dl pthread udev
ifneq ("$(OSTYPE)","Darwin")
	USED_LIBS += rt 
endif
CFLAGS += -Wall

include ../../BuildSystem/CommonCppMakefile
