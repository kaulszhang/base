PLATFORM_TOOL_NAME	:= gcc42

DYNAMIC_NAME_SUFFIX     := .dylib

CXX                     := clang++ -std=gnu++11 -stdlib=libc++

STRIP                   := strip -x

SHELL			:= /bin/bash

AR                      := libtool -static -arch_only x86_64 -o
