include ../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

BIN_DIR = ../../Bin

INC_DIRS = \
	. \
	../../Include \
	../../ThirdParty/PSCommon/XnLib/Include
	

SRC_FILES = \
	*.cpp \

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

SLIB_NAME = DepthUtils
ifneq ("$(OSTYPE)","Darwin")
	USED_LIBS += rt 
endif
CFLAGS += -Wall

include ../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
