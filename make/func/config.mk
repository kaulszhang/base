
include $(ROOT_MAKE_DIRECTORY)/func/base.mk

# 获取枚举类型的配置值
# argment1:	实际输入的配置参数
# argment2:	枚举定义的所有可能值
# argment3:	枚举定义的默认值
# return:	实际输入中第一个出现的枚举值或者默认值

get_config		= $(firstword $(strip $(foreach config,$(1),$(findstring $(config),$(2))) $(3)))

define handle_config
        $(if $(filter --enable-%,$(1)),CONFIG_$(1:--enable-%=%):=yes, \
        $(if $(filter --disable-%,$(1)),CONFIG_$(1:--disable-%=%):=no, \
        $(if $(filter --%,$(1)),$(patsubst --%,CONFIG_%,$(word 1,$(call split,=,$(1)))):=$(call split,:,$(patsubst $(word 1,$(call split,=,$(1)))=%,%,$(1))))))
endef

# 解析配置参数
# --enable-NAME 替换为 CONFIG_NAME:=yes
# --disable-NAME 替换为 CONFIG_NAME:=no
# --NAME=A:B:C 替换为 CONFIG_NAME:=A B C
# argment1:	实际输入的配置参数集
# return:

parse_config		= $(foreach config,$(call revert,$(1)),$(eval $(call handle_config,$(config))))
