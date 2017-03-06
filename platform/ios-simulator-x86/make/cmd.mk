PLATFORM_TOOL_PATH      := /toolchain /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin

PLATFORM_TOOL_NAME      := gcc42

PLATFORM_TOOL_PREFIX    := 

CXX                     := clang++ -std=gnu++11 -stdlib=libc++

CC                      := clang

STRIP                   := strip -x

SHELL                   := /bin/bash

AR                      := libtool -static -arch_only i386 -o

LT                      := libtool -static -arch_only i386

PLATFORM_TOOL_TARGET    := i386-apple-darwin
