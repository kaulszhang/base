PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -arch i386 -miphoneos-version-min=7.0
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -gdwarf-2 -DPPBOX_CONTANINER=DequeEx
PLATFORM_COMPILE_FLAGS		:= $(PLATFORM_COMPILE_FLAGS) -isysroot /Applications/Xcode.app/Contents/Developer/Platforms/iPhoneSimulator.platform/Developer/SDKs/iPhoneSimulator10.0.sdk

PLATFORM_LINK_FLAGS		:= $(PLATFORM_COMPILE_FLAGS)

PLATFORM_DISABLE_FLAGS		:= -Wl,-Os -Wl,--exclude-libs -Wl,-rpath rcs

PLATFORM_DEPEND_LIBRARYS_DIR    := ios

PLATFORM_DEPEND_LIBRARYS    := $(PLATFORM_DEPEND_LIBRARYS) ssl crypto
