#pragma once

#include "vulkan_common.h"

void vulkan_font_init(vk_context_t* vk);
void vulkan_font_destroy(VkDevice device);
void vulkan_font_update_assets(VkDevice device, VkCommandBuffer cmd);
void vulkan_font_render(VkCommandBuffer cmd);
