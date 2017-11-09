
TARGET = test
MODULE = module.a
#MODULE = gambatte/gambatte_module.a
#MODULE = snes9x/snes9x_module.a
DEBUG = 0

include base.mk


BUILD_DIR = objs/$(platform)

ifeq ($(DEBUG),1)
   BUILD_DIR := $(BUILD_DIR)-debug
endif

OBJS :=

OBJS += main.o
OBJS += console.o
OBJS += video.o
OBJS += ui/hitbox.o
OBJS += ui/slider.o
OBJS += ui/button.o
ifeq ($(platform),linux)
   OBJS += linux/platform.o
   OBJS += linux/audio_alsa.o
   OBJS += linux/input_x11.o
else ifeq ($(platform),win)
   OBJS += win/platform.o
   OBJS += win/audio.o
   OBJS += win/input.o

   OBJS += win/d3d9/main.o
   ifeq ($(HAVE_D3D10),1)
      OBJS += win/d3d10/main.o
   endif
   ifeq ($(HAVE_D3D11),1)
      OBJS += win/d3d11/main.o
   endif
   ifeq ($(HAVE_D3D12),1)
      OBJS += win/d3d12/main.o
   endif

endif
OBJS += gl/main.o
OBJS += gl/stubs.o
ifeq ($(HAVE_VULKAN),1)
   OBJS += utils/png_file.o
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
   HAS_SHADERS += vulkan/font.o
   HAS_SHADERS += vulkan/font_mono.o
   HAS_SHADERS += vulkan/frame.o
   HAS_SHADERS += vulkan/slider.o
   HAS_SHADERS += vulkan/sprite.o
endif
OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS))


HAS_SHADERS := $(basename $(HAS_SHADERS))
$(foreach obj,$(HAS_SHADERS),$(eval $(BUILD_DIR)/$(obj).o: $(obj).vert.inc $(obj).frag.inc $(obj).geom.inc))
$(foreach obj,$(HAS_SHADERS),$(eval SPIRV_OBJS += $(obj).vert.inc $(obj).frag.inc $(obj).geom.inc))

ifeq ($(DEBUG),1)
   CFLAGS += -g -O0 -DDEBUG
else
   CFLAGS += -g -O3 -DNDEBUG
endif

CFLAGS += -Wall -Werror=implicit-function-declaration -Werror=incompatible-pointer-types
CFLAGS += -Werror -Wno-error=unused-variable
CFLAGS += -fms-extensions
CFLAGS += -I.

ifeq ($(HAVE_VULKAN),1)
   CFLAGS += -DHAVE_VULKAN
   LIBS += -lvulkan
endif

ifeq ($(platform),linux)
   CFLAGS += -DVK_USE_PLATFORM_XLIB_KHR
   CFLAGS += -DVK_USE_PLATFORM_XLIB_XRANDR_EXT
   CFLAGS += -DHAVE_X11
   LIBS += -lX11 -lasound -lGL
   CFLAGS += $(shell freetype-config --cflags)
else ifeq ($(platform),win)
   ifneq ($(WindowsSDKVersion),)
      CFLAGS += -idirafter"$(WindowsSdkDir)include\$(WindowsSDKVersion)"
      LIBS +=  -L"$(WindowsSdkDir)lib\$(WindowsSDKVersion)\um\x64"
      CFLAGS += -DHAVE_D3D12
      LIBS +=  -ld3d12
#      CFLAGS += -idirafter"$(WindowsSdkDir)include\$(WindowsSDKVersion)ucrt"
#      CFLAGS += -idirafter"$(WindowsSdkDir)include\$(WindowsSDKVersion)shared"
#      CFLAGS += -idirafter"$(WindowsSdkDir)include\$(WindowsSDKVersion)um"
#      LIBS +=  -L"$(WindowsSdkDir)lib\$(WindowsSDKVersion)ucrt\x64"
#      LIBS +=  "$(WindowsSdkDir)lib\$(WindowsSDKVersion)um\x64\d3d12.lib"
   endif
   ifeq ($(HAVE_D3D10),1)
      CFLAGS += -DHAVE_D3D10
      LIBS +=  -ld3d10 -ldxgi -ld3dcompiler
   endif
   ifeq ($(HAVE_D3D11),1)
      CFLAGS += -DHAVE_D3D11
      LIBS +=  -ld3d11
   endif
   ifeq ($(HAVE_VULKAN),1)
      CFLAGS += -I$(VULKAN_SDK)/Include -DVK_USE_PLATFORM_WIN32_KHR
      LIBS +=  -L$(VULKAN_SDK)/Lib
   #   LIBS +=  -lvulkan-1
   endif
   LIBS +=  -ld3d9 -lopengl32
   CFLAGS += -DHAVE_D3D9
   CFLAGS += $(shell pkg-config.exe freetype2 --cflags)
   LIBS +=  -lgdi32 -ldinput -ldxguid -ldinput8 -ldsound

endif

LIBS += -lfreetype -lpng

#CFLAGS += -DPRINTF_WRAPPED
#LDFLAGS += -Wl,--wrap,printf

include build.mk
