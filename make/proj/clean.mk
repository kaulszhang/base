.PHONY: clean 
clean: $(if $(findstring auto,$(PROJECT_TYPE)),autoclean,)
	@$(ECHO) clean $(OBJECT_DIRECTORY) $(DEPEND_DIRECTORY) $(TARGET_FILE_FULL)
	@$(RM) $(TARGET_FILE_FULL) $(OBJECT_DIRECTORY) $(DEPEND_DIRECTORY)

.PHONY: autoclean
autoclean:
	@$(ECHO) distclean $(ROOT_SOURCE_DIRECTORY)$(LOCAL_NAME)
	@$(MAKE) -C $(ROOT_SOURCE_DIRECTORY)$(LOCAL_NAME) distclean
	@$(RM) $(TARGET_DIRECTORY)/*
