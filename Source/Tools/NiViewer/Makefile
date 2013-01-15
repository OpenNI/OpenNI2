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
	CFLAGS += MACOS
	LDFLAGS += -framework OpenGL -framework GLUT
else
	CFLAGS += -DUNIX -DGLX_GLXEXT_LEGACY
	USED_LIBS += glut GL
	LDFLAGS += -pthread
endif

LIB_DIRS  += ../../../ThirdParty/PSCommon/XnLib/Bin/$(PLATFORM)-$(CFG)
USED_LIBS += OpenNI2 XnLib

EXE_NAME = NiViewer

CFLAGS += -Wall

include ../../../ThirdParty/PSCommon/BuildSystem/CommonCppMakefile
