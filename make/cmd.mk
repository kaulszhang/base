
ifeq ($(AR),)
	AR		:= ar
endif

ifeq ($(AS),)
	AS		:= as
endif

ifeq ($(CPP),)
	CPP		:= cpp
endif

ifeq ($(CC),)
	CC		:= gcc
endif

ifeq ($(CXX),)
	CXX		:= g++
endif

ifeq ($(LD),ld)
	LD		:= $(CXX)
endif
ifeq ($(LD),)
	LD		:= $(CXX)
endif

ifeq ($(LT),)
	LT		:= libtool
endif

ifeq ($(STRIP),)
	STRIP		:= strip -s
endif

AR		:= $(PLATFORM_TOOL_PREFIX)$(AR)
AS		:= $(PLATFORM_TOOL_PREFIX)$(AS)
CPP		:= $(PLATFORM_TOOL_PREFIX)$(CPP)
CC		:= $(PLATFORM_TOOL_PREFIX)$(CC)
CXX		:= $(PLATFORM_TOOL_PREFIX)$(CXX)
LD		:= $(PLATFORM_TOOL_PREFIX)$(LD)
LT		:= $(PLATFORM_TOOL_PREFIX)$(LT)
STRIP		:= $(PLATFORM_TOOL_PREFIX)$(STRIP)

ifeq ($(CD),)
	CD	        := cd
endif

ifeq ($(RM),)
	RM		:= rm -fr
endif

ifeq ($(RM),rm -f)
	RM		:= rm -fr
endif

ifeq ($(LN),)
	LN		:= ln -f
endif

ifeq ($(CP),)
	CP		:= cp -f
endif

ifeq ($(MV),)
	MV		:= mv
endif

ifeq ($(MKDIR),)
	MKDIR		:= mkdir -p
endif

ifeq ($(ECHO),)
	ECHO		:= echo
endif

ifeq ($(SHELL),)	
	SHELL		:= /bin/sh
endif

ifeq ($(CTARGS),)
	CTARGS		:= ctags --extra=+fq \
	--c++-types=cfgmnpstu --append=yes 
endif

ifeq ($(CSCOPE_CMD),)
	CSCOPE_CMD	:= cscope -b
endif

ifeq ($(MAKE),)
	MAKE		:= make
endif

ifeq ($(SED),)
	SED		:= sed
endif

ifeq ($(EV),)
	EV		:= file_version
endif
