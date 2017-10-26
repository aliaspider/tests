
#include "vulkan_common.h"

extern vk_renderer_t slider_renderer;

void vk_slider_init(vk_context_t *vk);
void vk_slider_destroy(VkDevice device);
void vk_slider_add(int x, int y, int w, int h, float pos, float size);
