
include $(ROOT_MAKE_DIRECTORY)/func/base.mk

# ��ȡö�����͵�����ֵ
# argment1:	ʵ����������ò���
# argment2:	ö�ٶ�������п���ֵ
# argment3:	ö�ٶ����Ĭ��ֵ
# return:	ʵ�������е�һ�����ֵ�ö��ֵ����Ĭ��ֵ

get_config		= $(firstword $(strip $(foreach config,$(1),$(findstring $(config),$(2))) $(3)))

define handle_config
        $(if $(filter --enable-%,$(1)),CONFIG_$(1:--enable-%=%):=yes, \
        $(if $(filter --disable-%,$(1)),CONFIG_$(1:--disable-%=%):=no, \
        $(if $(filter --%,$(1)),$(patsubst --%,CONFIG_%,$(word 1,$(call split,=,$(1)))):=$(call split,:,$(patsubst $(word 1,$(call split,=,$(1)))=%,%,$(1))))))
endef

# �������ò���
# --enable-NAME �滻Ϊ CONFIG_NAME:=yes
# --disable-NAME �滻Ϊ CONFIG_NAME:=no
# --NAME=A:B:C �滻Ϊ CONFIG_NAME:=A B C
# argment1:	ʵ����������ò�����
# return:

parse_config		= $(foreach config,$(call revert,$(1)),$(eval $(call handle_config,$(config))))
