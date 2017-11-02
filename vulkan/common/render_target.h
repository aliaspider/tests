#pragma once

#include <stdlib.h>

#include "common.h"
#include "context.h"
#include "video.h"

#define MAX_SWAPCHAIN_IMAGES 8

typedef struct
{
   VkSurfaceKHR surface;
   screen_t *screen;
   VkDisplayKHR display;
   VkSwapchainKHR swapchain;
   VkRect2D scissor;
   VkViewport viewport;
   uint32_t swapchain_count;
   VkImageView views[MAX_SWAPCHAIN_IMAGES];
   VkFramebuffer framebuffers[MAX_SWAPCHAIN_IMAGES];
   VkRenderPassBeginInfo renderpass_info[MAX_SWAPCHAIN_IMAGES];
   VkCommandBuffer cmd;
   VkClearValue clear_value;
   VkFence chain_fence;
   vk_drawcmd_list_t *draw_list;
   bool vsync;
} vk_render_target_t;

void vk_render_targets_init(vk_context_t *vk, int count, screen_t *screens, vk_render_target_t *render_targets);
void vk_render_targets_destroy(vk_context_t *vk, int count, vk_render_target_t *render_targets);
void vk_swapchain_init(vk_context_t *vk, vk_render_target_t *render_target);
void vk_swapchain_destroy(vk_context_t *vk, vk_render_target_t *render_target);

