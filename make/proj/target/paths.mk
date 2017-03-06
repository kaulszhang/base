
INC_PATHS		:= $(INC_PATHS) $(wildcard $(addsuffix include,$(DEPEND_PATHS)))
INC_PATHS		:= $(INC_PATHS) $(HEADER_DIRECTORYS)
INC_PATHS		:= $(INC_PATHS) $(PLATFORM_DIRECTORY)/include

ifneq ($(CONFIG_LIB2),static) # dynamic static2, bin

LIB_PATHS		:= $(LIB_PATHS) $(DEPEND_PATHS)

ifneq ($(CONFIG_LIB2),static2) # dynamic, bin

LIB_PATHS		:= $(LIB_PATHS) $(PLATFORM_LIBRARY_DIRECTORYS)
LIB_PATHS		:= $(LIB_PATHS) $(PLATFORM_DIRECTORY)/lib

ifneq ($(PLATFORM_DEPEND_LIBRARYS_DIR),)
LIB_PATHS               := $(LIB_PATHS) $(ROOT_PLATFORM_DIRECTORY)/libs/$(PLATFORM_DEPEND_LIBRARYS_DIR)
endif


LIB_PATHS2		:= $(filter %/,$(DEPEND_FILES))
LIB_PATHS2		:= $(call joinlist,:,$(strip $(LIB_PATHS2) $(LD_LIBRARY_PATH)))

endif # dynamic, bin

endif # dynamic static2, bin
