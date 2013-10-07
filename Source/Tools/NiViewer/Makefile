include ../../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

BIN_DIR = ../../../Bin

INC_DIRS = \
	../../../Include \
	../../../ThirdParty/PSCommon/XnLib/Include \
	../../../ThirdParty/GL/

SRC_FILES = \
	Device.cpp  \
	Draw.cpp  \
	Keyboard.cpp  \
	Menu.cpp  \
	MouseInput.cpp  \
	NiViewer.cpp  \
	Statistics.cpp \
	Capture.cpp

ifeq ("$(OSTYPE)","Darwin")
	CFLAGS += -DMACOS
	LDFLAGS += -framework OpenGL -framework GLUT
else
	CFLAGS += -DUNIX -DGLX_GLXEXT_LEGACY
	USED_LIBS += glut GLU GL
endif

LIB_DIRS  += ../../../ThirdParty/PSCommon/XnLib/Bin/$(PLATFORM)-$(CFG)
USED_LIBS += OpenNI2 XnLib pthread
ifneq ("$(OSTYPE)","Darwin")
        USED_LIBS += rt 
endif


EXE_NAME = NiViewer

CFLAGS += -Wall

include ../../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
