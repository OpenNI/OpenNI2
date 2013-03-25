include ../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

BIN_DIR = ../../Bin

INC_DIRS = \
	../../Include \
	../../ThirdParty/GL/ \
	../Common

SRC_FILES = *.cpp

ifeq ("$(OSTYPE)","Darwin")
	CFLAGS += -DMACOS
	LDFLAGS += -framework OpenGL -framework GLUT
else
	CFLAGS += -DUNIX -DGLX_GLXEXT_LEGACY
	USED_LIBS += glut GL
endif

USED_LIBS += OpenNI2

EXE_NAME = SimpleViewer

CFLAGS += -Wall

include ../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
