
include $(ROOT_MAKE_DIRECTORY)/func/info.mk

# __为了防止重名
define make_item_depends_pack
$(2)            := $(strip $($(2)) __$(1)__)
endef

define enable_item_depends_pack
$(1)
$(eval $(call make_item_depends_pack,$(1),$(2)))
$(foreach item,$(1),$(call enable_item_depends_pack,$(filter-out $(strip $($(2))),$(call get_item_depends,$(item))),$(2)))
endef

# param 1 root 2 handle
define get_child
$(strip $(call get_item,$(1),Depends))
endef

define get_item_info_vaul
$(eval get_item_info_as_var_ret:=$(strip \
        $(shell $(MAKE) LOCAL_NAME=$(1) config="$(strip $(config))" info | \
                awk -F : ' 
                        BEGIN { $(foreach info,$(2),infos["$(info)"];);ii="$(1)"; } 
                        { if ($$1 in infos) printf "$$(eval %s_%s:=%s)", ii, $$1, $$2; } 
                ' \
        ) \
))$(get_item_info_as_var_ret)
endef


define get_item
$($(1)_$(2))
endef


#param 1 rootdir(/streamsdk/streamsdk)  2 list(Depends File) 3 fun(for print) 4 variable(key) 5 fromat 
define tree_visit
$(strip $(1) \
	$(eval $(call make_item_depends_pack,$(1),$(4))) \
	$(call get_item_info_vaul,$(1),$(2)) \
	$(eval $(call $(3),$(1),$(5))) \
	$(foreach item,$(call get_child,$(1)),$(if $(findstring __$(item)__,$($(4))),,$(call tree_visit,$(item),$(2),$(3),$(4),$(5)))) \
)
endef

