################################################################################
## @file: 	Makefile
## @author	郭春茂 <gcm.ustc.edu>
## @brief	构建tools目录.
## @version	1.0
###############################################################################

ROOT_PLATFORM_DIRECTORY	:= ../platform

ROOT_MAKE_DIRECTORY	:= ../make

ifeq ($(ROOT_TOOLS_DIRECTORY),)
        $(error Please define ROOT_TOOLS_DIRECTORY correctly!)
endif

PLATFORM_NAME_ALL	:= $(MAKECMDGOALS)

ifeq ($(PLATFORM_NAME_ALL),)
	PLATFORM_NAME_ALL	:= $(notdir $(wildcard $(ROOT_PLATFORM_DIRECTORY)/*))
endif

include $(ROOT_MAKE_DIRECTORY)/cmd.mk

include $(ROOT_MAKE_DIRECTORY)/func/dirs.mk

ifneq ($(words $(PLATFORM_NAME_ALL)),1)

.PHONY : all

all : $(PLATFORM_NAME_ALL)

.PHONY: $(PLATFORM_NAME_ALL)
$(PLATFORM_NAME_ALL) : 
	$(MAKE) ROOT_TOOLS_DIRECTORY=$(ROOT_TOOLS_DIRECTORY) $@

else

-include $(foreach platform,$(MAKECMDGOALS),$(ROOT_PLATFORM_DIRECTORY)/$(platform)/make/cmd.mk)

ROOT_TOOLS_DIRECTORY2	:= $(if $(filter /%,$(ROOT_TOOLS_DIRECTORY)),,../)$(ROOT_TOOLS_DIRECTORY)

$(PLATFORM_NAME_ALL) : $(addprefix $(PLATFORM_NAME_ALL),$(PLATFORM_TOOL_PATH))

.PHONY: $(addprefix $(PLATFORM_NAME_ALL),$(PLATFORM_TOOL_PATH))
$(addprefix $(PLATFORM_NAME_ALL),$(PLATFORM_TOOL_PATH)) : 
	-$(LN) -s $(ROOT_TOOLS_DIRECTORY2)/$(call root_directory,$@,2) $(PLATFORM_NAME_ALL)/

endif
