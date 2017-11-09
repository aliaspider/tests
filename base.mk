
all: build


platform ?= linux
#platform = win

ifeq ($(platform),linux)

   HAVE_VULKAN = 1
   HAVE_OPENGL = 1
endif

ifeq ($(platform),win)
   SPACE :=
   SPACE := $(SPACE) $(SPACE)
   BACKSLASH :=
   BACKSLASH := \$(BACKSLASH)
   filter_out1 = $(filter-out $(firstword $1),$1)
   filter_out2 = $(call filter_out1,$(call filter_out1,$1))

   reg_query = $(call filter_out2,$(subst $2,,$(shell reg query "$2" -v "$1" 2>nul)))
   fix_path = $(subst $(SPACE),\ ,$(subst \,/,$1))
   WindowsSdkDir := $(call reg_query,InstallationFolder,HKEY_LOCAL_MACHINE\SOFTWARE\Wow6432Node\Microsoft\Microsoft SDKs\Windows\v10.0)
   ifeq ($(WindowsSdkDir),)
      WindowsSdkDir := $(call reg_query,InstallationFolder,HKEY_CURRENT_USER\SOFTWARE\Wow6432Node\Microsoft\Microsoft SDKs\Windows\v10.0)
   endif
   ifeq ($(WindowsSdkDir),)
      WindowsSdkDir := $(call reg_query,InstallationFolder,HKEY_LOCAL_MACHINE\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v10.0)
   endif
   ifeq ($(WindowsSdkDir),)
      WindowsSdkDir := $(call reg_query,InstallationFolder,HKEY_CURRENT_USER\SOFTWARE\Microsoft\Microsoft SDKs\Windows\v10.0)
   endif
   WindowsSDKVersion := $(firstword $(foreach folder,$(subst $(subst \,/,$(WindowsSdkDir)Include/),,$(wildcard $(call fix_path,$(WindowsSdkDir)Include\*))),$(if $(wildcard $(call fix_path,$(WindowsSdkDir)Include/$(folder)/um/Windows.h)),$(folder),)))

#      VSINSTALLDIR ?= $(patsubst %Common7\Tools\,%,$(VS140COMNTOOLS))
#      VCINSTALLDIR ?= $(VSINSTALLDIR)VC$(BACKSLASH)
#      INCLUDE ?=$(VCINSTALLDIR)INCLUDE;$(VCINSTALLDIR)ATLMFC\INCLUDE;$(WindowsSdkDir)include\$(WindowsSDKVersion)ucrt;$(WindowsSdkDir)include\$(WindowsSDKVersion)shared;$(WindowsSdkDir)include\$(WindowsSDKVersion)um;
#      LIB ?=$(VCINSTALLDIR)LIB\amd64;$(VCINSTALLDIR)ATLMFC\LIB\amd64;$(WindowsSdkDir)lib\$(WindowsSDKVersion)ucrt\x64;$(WindowsSdkDir)lib\$(WindowsSDKVersion)um\x64;
#      LIBPATH ?=$(VCINSTALLDIR)LIB\amd64;$(VCINSTALLDIR)ATLMFC\LIB\amd64;

   ifneq ($(WindowsSDKVersion),)
      HAVE_D3D12 = 1
   endif


   HAVE_VULKAN = 1
   HAVE_OPENGL = 1
   HAVE_D3D10 = 1
   HAVE_D3D11 = 1

   EXT := .exe

endif


ifeq ($(platform),3ds)

BUILD_3DSX              = 1
BUILD_3DS               = 0
BUILD_CIA               = 1

APP_TITLE            = CTRRUN
APP_DESCRIPTION      = CTRRUN
APP_AUTHOR           = various
APP_PRODUCT_CODE     = CTRRUN
APP_UNIQUE_ID        = 0xBC100
APP_ICON             = 3ds/ctr/icon.png
APP_BANNER           = 3ds/ctr/banner.png
APP_AUDIO            = 3ds/ctr/silent.wav
APP_RSF              = 3ds/ctr/tools/template.rsf
APP_SYSTEM_MODE      = 64MB
APP_SYSTEM_MODE_EXT  = 124MB

ifeq ($(strip $(DEVKITPRO)),)
$(error "Please set DEVKITPRO in your environment. export DEVKITPRO=<path to>devkitpro")
endif
APP_TITLE         := $(shell echo "$(APP_TITLE)" | cut -c1-128)
APP_DESCRIPTION   := $(shell echo "$(APP_DESCRIPTION)" | cut -c1-256)
APP_AUTHOR        := $(shell echo "$(APP_AUTHOR)" | cut -c1-128)
APP_PRODUCT_CODE  := $(shell echo $(APP_PRODUCT_CODE) | cut -c1-16)
APP_UNIQUE_ID     := $(shell echo $(APP_UNIQUE_ID) | cut -c1-7)
MAKEROM_ARGS_COMMON = -rsf $(APP_RSF) -exefslogo -elf $(BUILD_DIR)/$(TARGET).elf -icon $(BUILD_DIR)/$(TARGET).icn -banner $(BUILD_DIR)/$(TARGET).bnr -DAPP_TITLE="$(APP_TITLE)" -DAPP_PRODUCT_CODE="$(APP_PRODUCT_CODE)" -DAPP_UNIQUE_ID=$(APP_UNIQUE_ID) -DAPP_SYSTEM_MODE=$(APP_SYSTEM_MODE) -DAPP_SYSTEM_MODE_EXT=$(APP_SYSTEM_MODE_EXT)
CFLAGS += -I$(DEVKITPRO)/libctru/include -I$(DEVKITPRO)/portlibs/armv6k/include
LIBDIRS := -L. -L$(DEVKITPRO)/libctru/lib -L $(DEVKITPRO)/portlibs/armv6k/lib

ARCH  := -march=armv6k -mtune=mpcore -mfloat-abi=hard -marm -mfpu=vfp -mtp=soft

CFLAGS += -mword-relocations -ffast-math -Werror=implicit-function-declaration $(ARCH)

CFLAGS += -Wall
CFLAGS += -DARM11 -D_3DS
ifeq ($(DEBUG), 1)
   CFLAGS   += -O0 -g
   LIBS     += -lctrud
else
   CFLAGS   += -O3 -fomit-frame-pointer
   LIBS     += -lctru
endif
ifeq ($(LIBCTRU_NO_DEPRECATION), 1)
   CFLAGS	+= -DLIBCTRU_NO_DEPRECATION
endif
CXXFLAGS := $(CFLAGS) -fno-rtti -fno-exceptions -std=gnu++11
ASFLAGS	:=  -g $(ARCH) -O3
LDFLAGS += -specs=3dsx.specs
LDFLAGS += -g $(ARCH) -Wl,-Map,$(notdir $*.map)
CFLAGS   += -std=gnu99 -ffast-math

LIBS += -lm -lz

#PATH := $(PATH):$(DEVKITPRO)/devkitARM/bin

CC      := arm-none-eabi-gcc
CXX     := arm-none-eabi-g++
AS      := arm-none-eabi-as
AR      := arm-none-eabi-ar
OBJCOPY := arm-none-eabi-objcopy
STRIP   := arm-none-eabi-strip
NM      := arm-none-eabi-nm
LD      := $(CXX)

ifneq ($(findstring Linux,$(shell uname -a)),)
	MAKEROM    = 3ds/ctr/tools/makerom-linux
	BANNERTOOL = 3ds/ctr/tools/bannertool-linux
else ifneq ($(findstring Darwin,$(shell uname -a)),)
	MAKEROM    = 3ds/ctr/tools/makerom-mac
	BANNERTOOL = 3ds/ctr/tools/bannertool-mac
else
	MAKEROM    = 3ds/ctr/tools/makerom.exe
	BANNERTOOL = 3ds/ctr/tools/bannertool.exe
endif

EXT := .elf
endif

$(info platform : $(platform))
