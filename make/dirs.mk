
include $(ROOT_MAKE_DIRECTORY)/cmd.mk

include $(ROOT_MAKE_DIRECTORY)/func/info.mk
include $(ROOT_MAKE_DIRECTORY)/proj/func/info.mk

TARGETS			:= target clean distclean info publish

MAKE_TARGETS		:= $(filter $(TARGETS),$(MAKECMDGOALS))

MAKE_DIRECTORYS		:= $(filter-out $(TARGETS),$(MAKECMDGOALS))

ifeq ($(SUBS),)
        SUBS			:= $(subst ^,*,$(patsubst %/,%,$(MAKE_DIRECTORYS)))
endif

ifeq ($(SUBS),)
	SUBS			:= $(addsuffix *,$(DIRECTORY_SUBS))
endif

ifeq ($(SUBS),)
        SUBS			:= *
endif

ifeq ($(findstring all-project,$(SUBS)),all-project)
	SUBS			:= $(DIRECTORY_SUBS) $(filter-out all-project,$(SUBS))
endif

SUBS1			:= $(SUBS)
SUBS1			:= $(foreach dir,$(SUBS1),$(wildcard $(ROOT_PROJECT_DIRECTORY)$(LOCAL_NAME)/$(dir)/Makefile.in))
SUBS1			:= $(patsubst $(ROOT_PROJECT_DIRECTORY)$(LOCAL_NAME)/%,%,$(SUBS1))
SUBS1			:= $(sort $(SUBS1))
SUBS1			:= $(patsubst %/,%,$(dir $(SUBS1)))
SUBS1			:= $(foreach dir,$(SUBS),$(findstring $(dir),$(SUBS1)))

SUBS			:= $(foreach dir,$(SUBS),$(wildcard $(dir)/Makefile))
SUBS			:= $(sort $(SUBS))
SUBS			:= $(patsubst %/,%,$(dir $(SUBS)) $(SUBS1))

OTHER_TARGETS		:= $(filter-out $(SUBS),$(MAKECMDGOALS))

OTHER_TARGETS_NO_INFO	:= $(filter-out info,$(OTHER_TARGETS))

ifeq ($(NO_RECURSION),yes)
        SUBS			:=
endif

.PHONY: $(TARGETS)
$(filter-out info,$(TARGETS)) : $(SUBS)

info:
	@$(ECHO) "Name: $(LOCAL_NAME)"
	@$(ECHO) "Type: $(LOCAL_TYPE)"
	@$(ECHO) "Subs: $(strip $(DIRECTORY_SUBS))"
	@$(ECHO) "Depends: $(addprefix $(LOCAL_NAME)/,$(strip $(DIRECTORY_SUBS)))"

.PHONY: $(OTHER_TARGETS_NO_INFO)
$(OTHER_TARGETS_NO_INFO): $(SUBS) 

.PHONY: $(SUBS)
$(SUBS): % : $(LOCAL_NAME)/%

define make_item_depends
$(1) : boot//$(PLATFORM_STRATEGY_NAME)$(1) $(2)
$(3)		:= $($(3)) __$(1)__

endef

include $(ROOT_MAKE_DIRECTORY)/boot.mk

define enable_item_depends
$(eval $(call make_item_depends,$(1),$(2),$(3)))
$(call make_code_boot,/$(PLATFORM_STRATEGY_NAME)$(1))
$(foreach item,$(2),$(if $(findstring __$(item)__,$($(3))),,$(call enable_item_depends,$(item),$(call get_item_depends,$(item)),$(3))))
endef

ifneq ($(MAKECMDGOALS),info)
        iiiii			:= $(call enable_item_depends,/VIRTUAL_TARGET,$(SUBS:%=$(LOCAL_NAME)/%),PROJECT_TARGETS)
endif

.PHONY: FORCE
FORCE:

/%: FORCE
	@echo $@
	$(MAKE) -C $(PLATFORM_BUILD_DIRECTORY)$@ NO_RECURSION=yes config="$(strip $(config))" $(MAKE_TARGETS)
	@echo

