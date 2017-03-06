PLATFORM_TOOL_PATH	:= /android-ndk-r10d/toolchains/aarch64-linux-android-4.9/prebuilt/linux-x86_64/bin

PLATFORM_TOOL_NAME	:= gcc49

PLATFORM_TOOL_PREFIX	:= aarch64-linux-android-

PLATFORM_TOOL_TARGET	:= arm-linux

#CXX                     := g++ -std=gnu++11
CXX                     := g++ -std=c++11 -fexceptions -frtti
#CXX                     := g++ -std=gnu++98

CC			:= gcc
AR			:= ar rcs
