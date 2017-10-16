
#include "vulkan_common.h"

void vulkan_font_init(VkDevice device, uint32_t queue_family_index, const VkMemoryType *memory_types, VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout,
                      const VkRect2D* scissor, const VkViewport* viewport, VkRenderPass renderpass);
void vulkan_font_destroy(VkDevice device, VkDescriptorPool pool);
void vulkan_font_update_assets(VkCommandBuffer cmd);
void vulkan_font_render(VkCommandBuffer cmd);
