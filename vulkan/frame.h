
#include "vulkan_common.h"

void vulkan_frame_init(VkDevice device, uint32_t queue_family_index, VkMemoryType *memory_types,
                  VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout,
                  int width, int height, VkFormat format,
                  const VkRect2D *scissor, const VkViewport *viewport, VkRenderPass renderpass);

void vulkan_frame_update(VkDevice device, VkCommandBuffer cmd);
void vulkan_frame_render(VkCommandBuffer cmd);
void vulkan_frame_destroy(VkDevice device);
