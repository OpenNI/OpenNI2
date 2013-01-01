include ../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

BIN_DIR = ../../Bin

INC_DIRS = \
	/usr/include/libfreenect \
	../../Source/Drivers/freenect \
	../../Include \
	../../ThirdParty/PSCommon/XnLib/Include

SRC_FILES = *.cpp

LIB_DIRS = ../../ThirdParty/PSCommon/XnLib/Bin/$(PLATFORM)-$(CFG)
USED_LIBS += OpenNI2 freenect XnLib

EXE_NAME = freenectTest

CFLAGS += -Wall

include ../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
