
include $(RELETIVE_DIR)/rootdir.mk

include $(ROOT_MAKE_DIRECTORY)/cmd.mk

include $(ROOT_MAKE_DIRECTORY)/func/config.mk

$(call parse_config,$(config))

PLATFORM_NAME_ALL	:= $(notdir $(wildcard $(ROOT_PLATFORM_DIRECTORY)/*))

MAKECMDGOALS_NO_SLASH	:= $(patsubst %/,%,$(MAKECMDGOALS))

PLATFORM_TARGETS	:= $(filter $(PLATFORM_NAME_ALL) all-platform,$(MAKECMDGOALS_NO_SLASH))

STRATEGY_TARGETS	:= $(filter $(addsuffix .%,$(PLATFORM_NAME_ALL)),$(MAKECMDGOALS_NO_SLASH))

OTHER_TARGETS		:= $(filter-out $(PLATFORM_TARGETS) $(STRATEGY_TARGETS),$(MAKECMDGOALS))

PASS_TARGETS		:= $(filter-out $(addsuffix /,$(PLATFORM_TARGETS) $(STRATEGY_TARGETS)),$(OTHER_TARGETS))

ifeq ($(PLATFORM_TARGETS)$(STRATEGY_TARGETS),)
	PLATFORM_TARGETS	:= $(wildcard *)
	STRATEGY_TARGETS	:= $(wildcard $(addsuffix .*,$(PLATFORM_NAME_ALL)))
endif

.PHONY: target
target: $(PLATFORM_TARGETS) $(STRATEGY_TARGETS)

.PHONY: all-platform
all-platform : $(PLATFORM_NAME_ALL)

.PHONY: $(PLATFORM_NAME_ALL)
$(PLATFORM_NAME_ALL) : % : boot//%$(if $(CONFIG_strategy),.$(CONFIG_strategy))
	@echo $@
ifneq ($(CONFIG_strategy),)
	@test -f $(ROOT_STRATEGY_DIRECTORY)/$(CONFIG_strategy).mk || (echo root.mk: no strategy $(CONFIG_strategy) && false)
endif
	$(MAKE) -C $@$(if $(CONFIG_strategy),.$(CONFIG_strategy)) config="$(strip $(config))" $(PASS_TARGETS)
	@echo

.PHONY: $(STRATEGY_TARGETS)
$(STRATEGY_TARGETS) : % : boot//%
	@echo $@
	@test -f $(ROOT_STRATEGY_DIRECTORY)/$(word 2,$(subst ., ,$(STRATEGY_TARGETS))).mk || (echo root.mk: no strategy $(word 2,$(subst ., ,$(STRATEGY_TARGETS))) && false)
	$(MAKE) -C $@ config="$(strip $(config))" $(PASS_TARGETS)
	@echo

.PHONY: $(OTHER_TARGETS)
$(OTHER_TARGETS) : $(PLATFORM_TARGETS) $(STRATEGY_TARGETS)

include $(ROOT_MAKE_DIRECTORY)/boot.mk

$(foreach platform,$(PLATFORM_NAME_ALL),$(call make_code_boot,/$(platform)))
