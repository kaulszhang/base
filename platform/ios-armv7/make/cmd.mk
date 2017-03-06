PLATFORM_TOOL_PATH	:= /toolchain /Applications/Xcode.app/Contents/Developer/Toolchains/XcodeDefault.xctoolchain/usr/bin 

PLATFORM_TOOL_NAME	:= gcc42

PLATFORM_TOOL_PREFIX	:= 

CXX                     := clang++ -std=gnu++11 -stdlib=libc++

CC                      := clang

STRIP                   := strip -x

SHELL                   := /bin/bash

AR                      := libtool -arch_only armv7 -static -o

LT                      := libtool -static -arch_only armv7

PLATFORM_TOOL_TARGET    := arm-apple-darwin 
