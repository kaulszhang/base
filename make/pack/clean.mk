.PHONY: clean
clean: 
	@$(ECHO) clean "$(addprefix $(TARGET_DIRECTORY)/,$(notdir $(DEPEND_FILES) $(PACKET_DEPEND_FILES))) $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)"
	@$(RM) $(addprefix $(TARGET_DIRECTORY)/,$(notdir $(PACKET_DEPEND_FILES))) $(TARGET_DIRECTORY)/$(PLATFORM_STRATEGY_NAME)