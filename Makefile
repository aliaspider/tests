
TARGET = test
MODULE = module.a
#MODULE = gambatte/gambatte_module.a
#MODULE = snes9x/snes9x_module.a
DEBUG = 0

platform = linux
#platform = win

BUILD_DIR = objs/$(platform)

ifeq ($(DEBUG),1)
   BUILD_DIR := $(BUILD_DIR)-debug
endif

all: $(TARGET)

OBJS :=

OBJS += main.o
OBJS += console.o
OBJS += ui/hitbox.o
OBJS += ui/slider.o
OBJS += ui/button.o
OBJS += utils/png_file.o
ifeq ($(platform),linux)
   OBJS += linux/platform.o
   OBJS += linux/audio_alsa.o
   OBJS += linux/input_x11.o
else ifeq ($(platform),win)
   OBJS += win/platform.o
   OBJS += win/audio.o
   OBJS += win/input.o

   OBJS += win/d3d9/main.o
   OBJS += win/d3d10/main.o
   OBJS += win/d3d11/main.o
   OBJS += win/d3d12/main.o

endif
OBJS += gl/main.o
OBJS += gl/stubs.o
OBJS += vulkan/console.o
OBJS += vulkan/font.o
OBJS += vulkan/font_mono.o
OBJS += vulkan/frame.o
OBJS += vulkan/main.o
OBJS += vulkan/slider.o
OBJS += vulkan/sprite.o
OBJS += vulkan/common/buffer.o
OBJS += vulkan/common/context.o
OBJS += vulkan/common/memory.o
OBJS += vulkan/common/render_target.o
OBJS += vulkan/common/renderer.o
OBJS += vulkan/common/resource.o
OBJS += vulkan/common/stubs.o
OBJS += vulkan/common/texture.o
OBJS += vulkan/common/utils.o

OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS))

HAS_SHADERS += vulkan/font.o
HAS_SHADERS += vulkan/font_mono.o
HAS_SHADERS += vulkan/frame.o
HAS_SHADERS += vulkan/slider.o
HAS_SHADERS += vulkan/sprite.o

HAS_SHADERS := $(basename $(HAS_SHADERS))
$(foreach obj,$(HAS_SHADERS),$(eval $(BUILD_DIR)/$(obj).o: $(obj).vert.inc $(obj).frag.inc $(obj).geom.inc))
$(foreach obj,$(HAS_SHADERS),$(eval SPIRV_OBJS += $(obj).vert.inc $(obj).frag.inc $(obj).geom.inc))

ifeq ($(DEBUG),1)
   CFLAGS += -g -O0 -DDEBUG
else
   CFLAGS += -g -O3 -DNDEBUG
endif

CFLAGS += -Wall -Werror=implicit-function-declaration -Werror=incompatible-pointer-types
CFLAGS += -Werror
CFLAGS += -fms-extensions
CFLAGS += -I.

ifeq ($(platform),linux)
   CFLAGS += -DVK_USE_PLATFORM_XLIB_KHR
   CFLAGS += -DVK_USE_PLATFORM_XLIB_XRANDR_EXT
   CFLAGS += -DHAVE_X11
   LIBS += -lvulkan -lX11 -lasound
   CFLAGS += $(shell freetype-config --cflags)
else ifeq ($(platform),win)
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
#   VSINSTALLDIR ?= $(patsubst %Common7\Tools\,%,$(VS140COMNTOOLS))
#   VCINSTALLDIR ?= $(VSINSTALLDIR)VC$(BACKSLASH)
#   INCLUDE ?=$(VCINSTALLDIR)INCLUDE;$(VCINSTALLDIR)ATLMFC\INCLUDE;$(WindowsSdkDir)include\$(WindowsSDKVersion)ucrt;$(WindowsSdkDir)include\$(WindowsSDKVersion)shared;$(WindowsSdkDir)include\$(WindowsSDKVersion)um;
#   LIB ?=$(VCINSTALLDIR)LIB\amd64;$(VCINSTALLDIR)ATLMFC\LIB\amd64;$(WindowsSdkDir)lib\$(WindowsSDKVersion)ucrt\x64;$(WindowsSdkDir)lib\$(WindowsSDKVersion)um\x64;
#   LIBPATH ?=$(VCINSTALLDIR)LIB\amd64;$(VCINSTALLDIR)ATLMFC\LIB\amd64;


   CFLAGS += -DHAVE_D3D9 -DHAVE_D3D12
   CFLAGS += $(shell pkg-config.exe freetype2 --cflags)
   CFLAGS += -I$(VULKAN_SDK)/Include -DVK_USE_PLATFORM_WIN32_KHR
   LIBS +=  -L"$(WindowsSdkDir)lib\$(WindowsSDKVersion)\um\x64" -ld3d9 -ld3d10 -ld3d11 -ld3d12 -ldxgi -ld3dcompiler -lopengl32
#   LIBS +=  "$(WindowsSdkDir)lib\$(WindowsSDKVersion)um\x64\d3d12.lib"
   LIBS +=  -L$(VULKAN_SDK)/Lib
#   LIBS +=  -lvulkan-1
   LIBS +=  -lvulkan
   LIBS +=  -lgdi32 -ldinput -ldxguid -ldinput8 -ldsound
#   CFLAGS += -idirafter"$(WindowsSdkDir)include\$(WindowsSDKVersion)ucrt"
#   CFLAGS += -idirafter"$(WindowsSdkDir)include\$(WindowsSDKVersion)shared"
   CFLAGS += -idirafter"$(WindowsSdkDir)include\$(WindowsSDKVersion)"
#   CFLAGS += -idirafter"$(WindowsSdkDir)include\$(WindowsSDKVersion)um"
#   LIBS +=  -L"$(WindowsSdkDir)lib\$(WindowsSDKVersion)ucrt\x64"

endif

LIBS += -lfreetype -lpng

#CFLAGS += -DPRINTF_WRAPPED
#LDFLAGS += -Wl,--wrap,printf

$(BUILD_DIR)/$(TARGET): $(OBJS) $(MODULE) .lastbuild
	touch .lastbuild
	$(CXX) $(OBJS) -L$(dir $(MODULE)) -l:$(notdir $(MODULE)) $(LDFLAGS) $(LIBDIRS) $(LIBS) -Wall -o $@

$(TARGET): $(BUILD_DIR)/$(TARGET)
	cp $< $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $< $(CFLAGS) -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.depend -c -o $@

%.vert.inc: %.vert
	glslc -c -mfmt=c $< -o $@

%.frag.inc: %.frag
	glslc -c -mfmt=c $< -o $@

%.geom.inc: %.geom
	glslc -c -mfmt=c $< -o $@

%.vert.inc: %.slang
	glslc -c -mfmt=c -fshader-stage=vertex -DVERTEX_SHADER $< -o $@

%.frag.inc: %.slang
	glslc -c -mfmt=c -fshader-stage=fragment -DFRAGMENT_SHADER $< -o $@

%.geom.inc: %.slang
	glslc -c -mfmt=c -fshader-stage=geometry -DGEOMETRY_SHADER $< -o $@

.lastbuild: ;

clean:
#	rm -rf objs
	rm -f $(OBJS) $(OBJS:.o=.depend)
	rm -f $(BUILD_DIR)/$(TARGET) $(TARGET) $(SPIRV_OBJS) .lastbuild


-include $(OBJS:.o=.depend)
