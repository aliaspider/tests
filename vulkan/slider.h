
#include "vulkan_common.h"

void vulkan_slider_init(vk_context_t *vk);
void vulkan_slider_destroy(VkDevice device);
void vulkan_slider_add(int x, int y, int w, int h, float pos, float size);
void vulkan_slider_start(void);
void vulkan_slider_update(VkDevice device, VkCommandBuffer cmd);
void vulkan_slider_finish(VkDevice device);
void vulkan_slider_render(VkCommandBuffer cmd);
