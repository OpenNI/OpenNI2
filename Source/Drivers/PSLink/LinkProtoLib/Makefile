include ../../../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

BIN_DIR = ../../../../Bin

INC_DIRS = \
	../../../../Include \
	../../../../ThirdParty/PSCommon/XnLib/Include \
	Include \
	./Protocols/XnLinkProto \

SRC_FILES = \
	*.cpp

CFLAGs += -Wall

LIB_DIRS = ../../../../ThirdParty/PSCommon/XnLib/Bin/$(PLATFORM)-$(CFG)
USED_LIBS = XnLib

SLIB_NAME = LinkProtoLib

include ../../../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
