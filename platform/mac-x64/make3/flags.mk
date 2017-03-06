TEMP1:=dylib_install_name,
LINK_FLAGS := $(subst soname=,$(TEMP1),$(LINK_FLAGS))
