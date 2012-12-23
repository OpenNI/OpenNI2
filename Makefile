#############################################################################
# OpenNI makefile.
# 
# default configuration is Release. for a debug version use:
# 	make CFG=Debug
#
# default compiler is g++. for another one use:
#   make CXX=<comp>
#
# By default, CLR projects will only be build if mono is installed.
# To force CLR projects use:
#   make FORCE_BUILD_CLR=1
#
#############################################################################

include ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

OPENNI = Source/Core
XNLIB  = ThirdParty/PSCommon/XnLib/Source

# list all drivers
ALL_DRIVERS = \
	Source/Drivers/DummyDevice   \
	Source/Drivers/PS1080 \
	Source/Drivers/OniFile

# list all tools
ALL_TOOLS = 

# list all core projects
ALL_CORE_PROJS = \
	$(XNLIB)  \
	$(OPENNI) \
	$(ALL_DRIVERS) \
	$(ALL_TOOLS)

# list all samples
CORE_SAMPLES = \
	Samples/SimpleRead \
	Samples/EventBasedRead \
	Samples/MultipleStreamRead \
	Samples/MWClosestPoint \
	Samples/MWClosestPointApp 
	
ifeq "$(GLUT_SUPPORTED)" "1"
	ALL_TOOLS += \
		Source/Tools/NiViewer

	CORE_SAMPLES += \
		Samples/SimpleViewer \
		Samples/MultiDepthViewer \
		Samples/ClosestPointViewer
else
	ifeq "$(GLES_SUPPORTED)" "1"
		CORE_SAMPLES += 
	endif
endif

ALL_SAMPLES = \
	$(CORE_SAMPLES)

# list all projects that are build
ALL_BUILD_PROJS = \
	$(ALL_CORE_PROJS) \
	$(ALL_SAMPLES)

ALL_PROJS = \
	$(ALL_BUILD_PROJS)

ALL_PROJS_CLEAN = $(foreach proj,$(ALL_PROJS),$(proj)-clean)

# define a function which creates a target for each proj
define CREATE_PROJ_TARGET
$1: 
	$$(MAKE) -C $1

$1-clean: 
	$$(MAKE) -C $1 clean
endef

################ TARGETS ##################

.PHONY: all $(ALL_PROJS) $(ALL_PROJS_CLEAN) install uninstall clean 

# make all makefiles
all: $(ALL_PROJS)

core: $(ALL_CORE_PROJS)

samples: $(ALL_SAMPLES)

# create projects targets
$(foreach proj,$(ALL_PROJS),$(eval $(call CREATE_PROJ_TARGET,$(proj))))

# additional dependencies
$(OPENNI):				  $(XNLIB)
Source/Drivers/DummyDevice:	$(OPENNI) $(XNLIB)
Source/Drivers/RawDevice:	$(OPENNI) $(XNLIB)
Source/Drivers/PS1080:		$(OPENNI) $(XNLIB)
Source/Drivers/OniFile:		$(OPENNI) $(XNLIB)

Source/Tools/NiViewer:		$(OPENNI) $(XNLIB)

Samples/SimpleRead:		$(OPENNI)
Samples/EventBasedRead:		$(OPENNI)
Samples/MultipleStreamRead:	$(OPENNI)
Samples/MWClosestPoint:		$(OPENNI)
Samples/MWClosestPointApp: 	$(OPENNI) Samples/MWClosestPoint

Samples/SimpleViewer:		$(OPENNI)
Samples/MultiDepthViewer:	$(OPENNI)
Samples/ClosestPointViewer:	$(OPENNI) Samples/MWClosestPoint

# clean is cleaning all projects
clean: $(ALL_PROJS_CLEAN)
