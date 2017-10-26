#pragma once

#include "vulkan_common.h"

extern vk_renderer_t frame_renderer;

void vk_frame_init(vk_context_t *vk, int width, int height, VkFormat format);
void vk_frame_add(int x, int y, int width, int height);
void vk_frame_destroy(VkDevice device);
