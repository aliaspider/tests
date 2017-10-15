
TARGET = test
#MODULE = gambatte/gambatte_module.a
MODULE = snes9x/snes9x_module.a
DEBUG = 0

platform = linux

BUILD_DIR = objs/$(platform)

ifeq ($(DEBUG),1)
   BUILD_DIR := $(BUILD_DIR)-debug
endif

all: $(TARGET)

OBJS :=

OBJS += main.o
OBJS += linux/platform.o
OBJS += linux/audio_alsa.o
OBJS += linux/input_x11.o
OBJS += vulkan/buffer.o
OBJS += vulkan/descriptors.o
OBJS += vulkan/device.o
OBJS += vulkan/instance.o
OBJS += vulkan/main.o
OBJS += vulkan/memory.o
OBJS += vulkan/pipeline.o
OBJS += vulkan/surface.o
OBJS += vulkan/swapchain.o
OBJS += vulkan/texture.o

OBJS := $(addprefix $(BUILD_DIR)/,$(OBJS))


$(BUILD_DIR)/vulkan/main.o: vulkan/main.vert.inc vulkan/main.frag.inc

ifeq ($(DEBUG),1)
   CFLAGS += -g -O0
else
   CFLAGS += -O3
endif

CFLAGS += -Wall -Werror=implicit-function-declaration -Werror=incompatible-pointer-types
CFLAGS += -Werror
CFLAGS += -DVK_USE_PLATFORM_XLIB_KHR
CFLAGS += -DVK_USE_PLATFORM_XLIB_XRANDR_EXT
CFLAGS += -DHAVE_X11
CFLAGS += -fms-extensions
CFLAGS += -I. -Ivulkan

LIBS += -lvulkan -lX11 -lasound


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

.lastbuild: ;

clean:
#	rm -rf objs
	rm -f $(OBJS) $(OBJS:.o=.depend)
	rm -f $(BUILD_DIR)/$(TARGET) $(TARGET) vulkan/main.vert.inc vulkan/main.frag.inc .lastbuild


-include $(OBJS:.o=.depend)
