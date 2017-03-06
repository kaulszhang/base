
ifeq ($(BUILDABLE)$(CONFIG_build_version),yesyes)

MAKE_VERSION_FILE	:= $(TARGET_DIRECTORY)/file.version

HAS_EV                  := $(shell if [ -f "/usr/local/bin/$(EV)" ];then echo "exist"; else echo "noexist";fi; )


ifeq ($(HAS_EV),noexist)
EV_CMD                  := 
else
EV_CMD                  := /usr/local/bin/$(EV) $(TARGET_FILE_FULL) $(VERSION_NAME) $(VERSION).$(BUILD_VERSION)
endif

target: $(MAKE_VERSION_FILE)

$(MAKE_VERSION_FILE): $(TARGET_FILE_FULL) $(BUILD_VERSION_FILE)
	@touch -r $(TARGET_FILE_FULL) $@
	$(EV_CMD)
	@touch -r $@ $(TARGET_FILE_FULL)
	@echo $(VERSION).$(BUILD_VERSION) > $@

endif
