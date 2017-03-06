
include $(ROOT_MAKE_DIRECTORY)/cmd.mk
include $(ROOT_MAKE_DIRECTORY)/name.mk

include $(ROOT_MAKE_DIRECTORY)/func/info.mk

PACK_MAKE_DIRECTORY     := $(ROOT_MAKE_DIRECTORY)/pack

include $(PACK_MAKE_DIRECTORY)/config.mk


TARGETS                 := target clean distclean info publish

ifeq ($(MAKECMDGOALS),)
        MAKECMDGOALS            := target
endif

include $(patsubst %,$(PACK_MAKE_DIRECTORY)/%.mk,$(filter $(TARGETS),$(MAKECMDGOALS)))

