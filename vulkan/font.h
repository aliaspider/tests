#pragma once

#include "vulkan_common.h"

void vulkan_font_init(vk_context_t* vk);
void vulkan_font_destroy(VkDevice device);
void vulkan_font_update_assets(VkDevice device, VkCommandBuffer cmd);
void vulkan_font_render(VkCommandBuffer cmd);
void vulkan_font_draw_text(const char *text, int x, int y, int max_width);
const char** vulkan_font_get_lines(const char *text, int x, int y, int max_width);
