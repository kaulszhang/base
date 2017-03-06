PLATFORM_COMPILE_FLAGS	:= -m64

PLATFORM_LINK_FLAGS	:= -m64

PLATFORM_DISABLE_FLAGS          := -Wl,-Os -Wl,--exclude-libs -Wl,-rpath -Wl,--version-script rcs
