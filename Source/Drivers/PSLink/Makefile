#############################################################################
# PrimeClient makefile.
# 
# default configuration is Release. for a debug version use:
# 	make CFG=Debug
#
# default compiler is g++. for another one use:
#   make CXX=<comp>
#
#############################################################################

include ../../../ThirdParty/PSCommon/BuildSystem/CommonDefs.mak

XNLIB  = ../../../ThirdParty/PSCommon/XnLib/Source

ALL_PROJS = \
	$(XNLIB)  \
	LinkProtoLib \
	PSLink \
	PSLinkConsole \
	
ALL_PROJS_CLEAN = $(foreach proj,$(ALL_PROJS),$(proj)-clean)

# define a function which creates a target for each proj
define CREATE_PROJ_TARGET
$1: 
	+$(MAKE) -C $1

$1-clean: 
	+$(MAKE) -C $1 clean
endef

################ TARGETS ##################

.PHONY: all $(ALL_PROJS) $(ALL_PROJS_CLEAN) install uninstall clean 

# make all makefiles
all: $(ALL_PROJS)

# create projects targets
$(foreach proj,$(ALL_PROJS),$(eval $(call CREATE_PROJ_TARGET,$(proj))))

# additional dependencies
LinkProtoLib:	$(XNLIB)
PSLink:			$(XNLIB) LinkProtoLib
PSLinkConsole:	$(XNLIB) LinkProtoLib

# clean is cleaning all projects
clean: $(ALL_PROJS_CLEAN)
