
include $(ROOT_MAKE_DIRECTORY)/func/macro.mk

ifeq ($(PROJECT_VERSION),)
        ifneq ($(PROJECT_VERSION_HEADER),)
		VERSION			:= $(call get_macro_info,$(PROJECT_VERSION_HEADER),VERSION)

                ifeq ($(VERSION),)
                        $(error VERSION not defined in $(PROJECT_VERSION_HEADER))
                endif
	
		VERSION_NAME		:= $(call get_macro_info,$(PROJECT_VERSION_HEADER),NAME)
        else
		CONFIG_build_version	:= no
        endif
else
	CONFIG_build_version	:= no
	VERSION			:= $(PROJECT_VERSION)
endif

NAME_VERSION		:= $(VERSION)

ifneq ($(NAME_VERSION),)
	NAME_VERSION		:= -$(NAME_VERSION)
endif

ifeq ($(BUILDABLE)$(CONFIG_build_version),yesyes)

        ifeq ($(VERSION_NAME),)
                $(error VERSION_NAME not defined in $(VERSION_HEADER))
        endif
	
	BUILD_VERSION		:= $(shell LANG=C svn info $(SOURCE_DIRECTORY) | awk -F : '$$1 == "Revision" { print $$2}' 2> /dev/null)

	BUILD_VERSION		:= $(strip $(BUILD_VERSION))

	BUILD_VERSION_FILE	:= build_version

        ifneq ($(BUILD_VERSION),$(shell cat $(BUILD_VERSION_FILE) 2> /dev/null))
                $(shell echo -n $(BUILD_VERSION) > $(BUILD_VERSION_FILE))
        endif

endif
