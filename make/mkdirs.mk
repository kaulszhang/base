.PHONY: mkdirs
mkdirs: $(MAKE_DIRECTORYS)

$(MAKE_DIRECTORYS):
	$(MKDIR) $@
