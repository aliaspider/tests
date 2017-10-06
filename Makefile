
TARGET = test
DEBUG = 1

all: $(TARGET)

OBJS :=

OBJS += main.o
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


vulkan/main.o: vulkan/main.vert.inc vulkan/main.frag.inc


CFLAGS += -Wall -Werror -Werror=implicit-function-declaration -Werror=incompatible-pointer-types
CFLAGS += -DVK_USE_PLATFORM_XLIB_KHR
CFLAGS += -I. -Ivulkan

ifeq ($(DEBUG),1)
CFLAGS += -g -O0
else
CFLAGS += -O3
endif

LIBS += -lvulkan -lX11 -lpng


$(TARGET): $(OBJS)
	$(CC) $(OBJS) $(LDFLAGS) $(LIBDIRS) $(LIBS) -Wall -o $@

%.o: %.c
	$(CC) $< $(CFLAGS) -c -o $@

%.vert.inc: %.vert
	glslc -c -mfmt=c $< -o $@

%.frag.inc: %.frag
	glslc -c -mfmt=c $< -o $@

clean:
	rm -f $(TARGET) $(OBJS) vulkan/main.vert.inc vulkan/main.frag.inc


