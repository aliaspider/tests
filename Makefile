
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
ifeq ($(platform),linux)
   OBJS += linux/platform.o
   OBJS += linux/audio_alsa.o
   OBJS += linux/input_x11.o
else ifeq ($(platform),win)
   OBJS += win/platform.o
   OBJS += win/audio.o
   OBJS += win/input.o
endif
OBJS += vulkan/console.o
OBJS += vulkan/font.o
OBJS += vulkan/font_mono.o
OBJS += vulkan/frame.o
OBJS += vulkan/main.o
OBJS += vulkan/resource.o
OBJS += vulkan/slider.o
OBJS += vulkan/sprite.o
OBJS += vulkan/vulkan_common.o
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
   CFLAGS += -g -O3
endif

CFLAGS += -Wall -Werror=implicit-function-declaration -Werror=incompatible-pointer-types
CFLAGS += -Werror
CFLAGS += -fms-extensions
CFLAGS += -I.

ifeq ($(platform),linux)
   CFLAGS += -DVK_USE_PLATFORM_XLIB_KHR
   CFLAGS += -DVK_USE_PLATFORM_XLIB_XRANDR_EXT
   CFLAGS += -DHAVE_X11
   LIBS += -lvulkan -lX11 -lasound -lfreetype
   CFLAGS += $(shell freetype-config --cflags)
else ifeq ($(platform),win)
   CFLAGS += $(shell pkg-config.exe freetype2 --cflags)
   CFLAGS += -I$(VULKAN_SDK)/Include -DVK_USE_PLATFORM_WIN32_KHR
   LIBS +=  -L$(VULKAN_SDK)/Lib -lvulkan-1 -lfreetype -lgdi32 -ldinput -ldxguid -ldinput8 -ldsound
endif

$(BUILD_DIR)/$(TARGET): $(OBJS) $(MODULE) .lastbuild
	touch .lastbuild
	$(CXX) $(OBJS) -L$(dir $(MODULE)) -l:$(notdir $(MODULE)) $(LDFLAGS) $(LIBDIRS) $(LIBS) -Wall -o $@

$(TARGET): $(BUILD_DIR)/$(TARGET)
	cp $< $@

$(BUILD_DIR)/%.o: %.c Makefile
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
