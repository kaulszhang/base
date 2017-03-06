
.PHONY: info
info:

include $(ROOT_MAKE_DIRECTORY)/func/dirs.mk

DEPENDS			:= $(PROJECT_DEPENDS) $(PLATFORM_DEPENDS)

DEPENDS1		:= $(filter /%,$(DEPENDS))

DEPENDS2		:= $(filter-out /%,$(DEPENDS))

DEPENDS2		:= $(call reletive_directory,$(DEPENDS2),$(LOCAL_NAME))

info:
	@$(ECHO) "Name: $(LOCAL_NAME)"
	@$(ECHO) "Type: $(LOCAL_TYPE)-$(PROJECT_TYPE2)$(if $(findstring lib,$(PROJECT_TYPE2)),-$(CONFIG_LIB))"
	@$(ECHO) "Config: $(CONFIG_PROFILE) $(CONFIG_THREAD) $(CONFIG_LIB)"
	@$(ECHO) "SourceDirectory: $(SOURCE_DIRECTORY)"
	@$(ECHO) "HeaderDirectory: $(HEADER_DIRECTORY)"
	@$(ECHO) "Buildable: $(BUILDABLE)"
	@$(ECHO) "TargetDirectory: $(TARGET_DIRECTORY)"
	@$(ECHO) "Target: $(PROJECT_TARGET)"
	@$(ECHO) "Version: $(VERSION)"
	@$(ECHO) "File: $(TARGET_FILE_FULL)"
	@$(ECHO) "Depends: $(DEPENDS1) $(DEPENDS2)"
	@$(ECHO) "DependLibs: $(PROJECT_DEPEND_LIBRARYS)"
