
.PHONY: info
info: 
	@$(ECHO) "Name: $(LOCAL_NAME)"
	@$(ECHO) "Type: $(LOCAL_TYPE)"
	@$(ECHO) "Config: $(CONFIG_PROFILE)"
	@$(ECHO) "Directory: $(TARGET_DIRECTORY)"
	@$(ECHO) "Target: $(PACKET_TARGET)"
	@$(ECHO) "Version: $(VERSION)"
	@$(ECHO) "File: $(TARGET_FILE_FULL)"
	@$(ECHO) "Depends: $(PACKET_DEPENDS)"
