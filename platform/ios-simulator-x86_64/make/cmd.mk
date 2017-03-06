PLATFORM_TOOL_PATH      := /toolchain /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin

PLATFORM_TOOL_NAME      := gcc42

PLATFORM_TOOL_PREFIX    := 

CXX                     := clang++ -std=gnu++11 -stdlib=libc++

CC                      := clang

STRIP                   := strip -x

SHELL                   := /bin/bash

AR                      := libtool -static -arch_only x86_64 -o

LT                      := libtool -static -arch_only x86_64

PLATFORM_TOOL_TARGET    := x86_64-apple-darwin
