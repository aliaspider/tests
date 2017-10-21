#pragma once

#include "vulkan_common.h"

void vulkan_frame_init(vk_context_t *vk, vk_render_context_t* render_contexts, int render_contexts_count, int width, int height, VkFormat format);
void vulkan_frame_update(VkDevice device, VkCommandBuffer cmd);
void vulkan_frame_render(VkCommandBuffer cmd);
void vulkan_frame_destroy(VkDevice device);
