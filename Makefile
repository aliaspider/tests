
TARGET = test
DEBUG = 1

BUILD_DIR = objs/linux

all: $(TARGET)

OBJS :=

OBJS += main.o
OBJS += linux/display.o
OBJS += linux/platform.o
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


CFLAGS += -Wall -Werror -Werror=implicit-function-declaration -Werror=incompatible-pointer-types
CFLAGS += -DVK_USE_PLATFORM_XLIB_KHR -DHAVE_X11
CFLAGS += -I. -Ivulkan

ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -O3
endif

LIBS += -lvulkan -lX11

$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBDIRS) $(LIBS) -Wall -o $@

$(BUILD_DIR)/%.o: %.c
	@mkdir -p $(dir $@)
	$(CC) $< $(CFLAGS) -MT $@ -MMD -MP -MF $(BUILD_DIR)/$*.depend -c -o $@

%.vert.inc: %.vert
	glslc -c -mfmt=c $< -o $@

%.frag.inc: %.frag
	glslc -c -mfmt=c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) $(OBJS:.o=.depend) vulkan/main.vert.inc vulkan/main.frag.inc


-include $(OBJS:.o=.depend)
