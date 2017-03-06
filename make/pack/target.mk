
FULL_TARGE_DIR               := $(PLATFORM_STRATEGY_NAME)_symbol
MAKE_DIRECTORYS         := $(TARGET_DIRECTORY) $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME) $(TARGET_DIRECTORY)/$(FULL_TARGE_DIR)

ifneq ($(CONFIG_packet),)
include $(ROOT_STRATEGY_DIRECTORY)/packet/$(CONFIG_packet).mk
endif

ifneq ($(CONFIG_extend),)
	include $(ROOT_STRATEGY_DIRECTORY)/extend/$(CONFIG_extend).mk
endif

call_post_action	= $(foreach action,$(2),$(call $(action),$(1)) && )true

.PHONY: target
target: $(TARGET_FILE_FULL)

include $(ROOT_MAKE_DIRECTORY)/mkdirs.mk

.PHONY: $(PACKET_DEPEND_FILES)
$(PACKET_DEPEND_FILES): mkdirs
	@$(ECHO) $@
	@$(RM) $(TARGET_DIRECTORY)/$(notdir $@)
	@svn export $(ROOT_DIRECTORY)$@ $(TARGET_DIRECTORY)/$(notdir $@) > /dev/null

.PHONY: $(PACKET_DEPEND_FILES2)
$(PACKET_DEPEND_FILES2): mkdirs
	@$(ECHO) $@
	@$(RM) $(TARGET_DIRECTORY)/$(notdir $@)
	$(CP) -r $(PLATFORM_BUILD_DIRECTORY)$@ $(TARGET_DIRECTORY)/$(notdir $@) > /dev/null

define packet_depend3
$(if $(findstring dynamic,$(call get_item_type,$(1))), \
	$(if $(findstring .so,$(DYNAMIC_NAME_SUFFIX)), \
		ln -f -s $(notdir $(call get_item_file,$(1))) $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)/libstreamsdk-$(PLATFORM_NAME)$(DYNAMIC_NAME_SUFFIX),) ,)
endef

.PHONY: EXTEND
EXTEND:
	$(call packet_depend3,/streamsdk/streamsdk)
	touch $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)/$(TARGET_FILE_3)

.PHONY: CLEAN
CLEAN:
	$(RM) $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)/*
	$(RM) $(TARGET_DIRECTORY)/$(FULL_TARGE_DIR)/*

define pack_depend
$(call pack_depend2,$(1),$(call get_item_type,$(1)),$(call get_item_file,$(1)))
endef

# pack_depend2 name type  file
define pack_depend2
$(if $(findstring static,$(2)), \
    $(CP) $(PLATFORM_BUILD_DIRECTORY)$(1)/$(3) $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)/$(notdir $(3)), \
    $(CP) $(PLATFORM_BUILD_DIRECTORY)$(1)/$(3) $(TARGET_DIRECTORY)/$(FULL_TARGE_DIR)/$(notdir $3) && $(STRIP) $(PLATFORM_BUILD_DIRECTORY)$(1)/$(3) -o $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)/$(notdir $(3))) && \
$(call call_post_action,$(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)/$(notdir $(3)),$(PACKET_POST_ACTION))
endef

.PHONY: $(PACKET_DEPENDS)
$(PACKET_DEPENDS): mkdirs
	@$(ECHO) $@
	$(call pack_depend,$@)

$(info TARGET_FILE_2=$(TARGET_FILE_2))
$(TARGET_FILE_FULL): CLEAN $(PACKET_DEPENDS) $(PACKET_DEPEND_FILES) $(PACKET_DEPEND_FILES2) $(MAKEFILE_LIST) EXTEND
	$(CD) $(TARGET_DIRECTORY) ; tar -czv -f $(TARGET_FILE_3) $(FULL_TARGE_DIR)
	$(CD) $(TARGET_DIRECTORY) ; tar -czv -f $(TARGET_FILE_2) $(PLATFORM_STRATEGY_NAME) $(notdir $(PACKET_DEPEND_FILES)) $(notdir $(PACKET_DEPEND_FILES2))
