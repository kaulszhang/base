PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -arch armv7
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -gdwarf-2 -DPPBOX_CONTANINER=DequeEx -Wundefined-bool-conversion -Wunused-local-typedef -Wunused-local-typedef
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneOS.platform/Developer/SDKs/iPhoneOS10.0.sdk

PLATFORM_LINK_FLAGS		:= $(PLATFORM_COMPILE_FLAGS)

PLATFORM_DISABLE_FLAGS		:= -Wl,-Os -Wl,--exclude-libs -Wl,-rpath rcs

PLATFORM_DEPEND_LIBRARYS_DIR    := ios

PLATFORM_DEPEND_LIBRARYS    := $(PLATFORM_DEPEND_LIBRARYS) ssl crypto
