
include $(RELETIVE_DIR)/rootdir.mk

include $(ROOT_MAKE_DIRECTORY)/cmd.mk

include $(ROOT_MAKE_DIRECTORY)/func/dirs.mk

SVN_ROOT		:= $(shell LANG=C svn info .. | awk -F : '$$1 == "URL" { print $$2$$3}' | sed "s/http\/\/[\.0-9a-z]*\//\//g" 2>/dev/null)

ifeq ($(SOURCE_BRANCH)1$(BRANCH_VERSION)2$(CODE_DIRECTORY),12)
        $(error Please define SOURCE_BRANCH BRANCH_VERSION CODE_DIRECTORY correctly!)
endif

PRIVATE_DIRECTORIES	:= src include lib bin pack pub conf

CODE_DIRECTORY_PARENTS	:= $(call revert,$(call root_directories,/$(CODE_DIRECTORY)))

PRIVATE_DIRECTORIES	:= $(PRIVATE_DIRECTORIES) $(foreach dir,$(CODE_DIRECTORY_PARENTS),src$(dir) include$(dir))

LINK_DIRECTORIES	:= /src /include $(foreach dir,$(CODE_DIRECTORY_PARENTS),/src$(dir) /include$(dir))

define add_leof
$(1)

endef

link_targets		= $(foreach dir,$(filter-out $(PRIVATE_DIRECTORIES),$(patsubst ../$(SOURCE_BRANCH)/%,%,$(wildcard ../$(SOURCE_BRANCH)$(1)/*))),$(shell echo $(SVN_ROOT)/$(SOURCE_BRANCH)/$(dir) $(notdir $(dir)) >> link_targets))

all: $(BRANCH_VERSION) $(PRIVATE_DIRECTORIES) $(LINK_DIRECTORIES)

$(BRANCH_VERSION) : 
	svn mkdir $(BRANCH_VERSION)
	svn ps --file link_targets svn:externals $@ $(call link_targets,)
	@$(RM) link_targets

.PHONY : $(PRIVATE_DIRECTORIES)
$(PRIVATE_DIRECTORIES) :
	svn mkdir $(BRANCH_VERSION)/$@

.PHONY : $(LINK_DIRECTORIES)
$(LINK_DIRECTORIES) :
	svn ps --file link_targets svn:externals $(BRANCH_VERSION)/$@ $(call link_targets,$@)
	@$(RM) link_targets

