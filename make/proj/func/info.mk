
# 提取工程项目目标文件
# argment1:	工程项目名称
# return:	工程项目目标文件

define get_project_file
$(PLATFORM_BUILD_DIRECTORY)$(1)/$(call get_item_info,$(1),File)
endef

# 递归遍历树（深度优先）
# argment1:	根节点
# argment2:	获取子节点的函数

#define tree_visit
#$(foreach child,$(call $(2),$(1)),$(call tree_visit,$(child),$(2)))
#endef
#
#define get_project_depend_order
#$(eval 
#	item		:= $(1)
#	items		:= $$(strip $$(items) $$(item))
#	vars		:= $$(call get_item_info_as_var,$$(item),Type Depends,_1)
#	depends		:= $$(Depends_1)
#	all_depends	:= $$(filter-out $$(depends),$$(all_depends)) $$(depends)
#	ret		:= $$(filter-out $$(items),$$(depends))
#)$(ret)$(warning get_project_depend_order($(1))=$(ret))
#endef
