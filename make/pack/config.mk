CONFIG                  := $(config) $(PACKET_CONFIG)

get_config              = $(firstword $(strip $(foreach config,$(1),$(findstring $(config),$(2))) $(3)))

CONIFG_COMPILE          := $(call get_config,$(CONFIG),debug release,debug)

TARGET_DIRECTORY        := $(CONIFG_COMPILE)

DEPENDS                 := $(PACKET_DEPENDS)

config                  := $(CONFIG)


VERSION_DEPEND          := $(call get_item_info,$(PACKET_VERSION_DEPEND),File)
VERSION_DEPEND          := $(PLATFORM_BUILD_DIRECTORY)$(PACKET_VERSION_DEPEND)/$(VERSION_DEPEND) 

VERSION                 := $(strip $(shell $(EV) $(VERSION_DEPEND) $(call get_item_info,$(PACKET_VERSION_DEPEND),Target)))

ifeq ($(VERSION),)
VERSION_SOURCE          := $(strip $(call get_item_info,$(PACKET_VERSION_DEPEND),SourceDirectory))
BUILD_VERSION           := $(strip $(shell LANG=C svn info $(VERSION_SOURCE) | awk -F : '$$1 == "Revision" { print $$2}' 2> /dev/null))
VERSION                 := $(PACKET_VERSION)
ifneq ($(BUILD_VERSION),)
VERSION                 := $(VERSION).$(BUILD_VERSION)
endif
endif

CUR_DATE                := $(shell date '+%Y%m%d%H%M%S')

ifneq ($(CONFIG_packet),)
TARGET_FILE             := $(PACKET_TARGET)-$(subst .,-,$(PLATFORM_STRATEGY_NAME))-$(CONIFG_COMPILE)($(CONFIG_packet))-$(VERSION).tar.gz
TARGET_FILE_2		:= $(PACKET_TARGET)-$(subst .,-,$(PLATFORM_STRATEGY_NAME))-$(CONIFG_COMPILE)\($(CONFIG_packet)\)-$(VERSION).tar.gz
TARGET_FILE_3		:= $(PACKET_TARGET)-$(subst .,-,$(PLATFORM_STRATEGY_NAME))-$(CONIFG_COMPILE)\($(CONFIG_packet)\)-$(VERSION)-$(CUR_DATE)-symbol.tar.gz
else
TARGET_FILE             := $(PACKET_TARGET)-$(subst .,-,$(PLATFORM_STRATEGY_NAME))-$(CONIFG_COMPILE)-$(VERSION).tar.gz
TARGET_FILE_2		:= $(TARGET_FILE)
TARGET_FILE_3		:= $(PACKET_TARGET)-$(subst .,-,$(PLATFORM_STRATEGY_NAME))-$(CONIFG_COMPILE)-$(VERSION)-$(CUR_DATE)-symbol.tar.gz
endif

TARGET_FILE_FULL        := $(TARGET_DIRECTORY)/$(TARGET_FILE)

PACKET_DEPEND_FILES     := $(shell $(ECHO) $(PACKET_DEPEND_FILES) | iconv -f gbk)

