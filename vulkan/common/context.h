#pragma once

#include "common.h"

typedef struct vk_context_t
{
   VkInstance instance;
   VkDebugReportCallbackEXT debug_cb;
   VkPhysicalDevice gpu;
   union
   {
      struct
      {
         uint32_t memoryTypeCount;
         VkMemoryType memoryTypes[VK_MAX_MEMORY_TYPES];
      };
      VkPhysicalDeviceMemoryProperties mem;
   };
   VkDevice device;
   uint32_t queue_family_index;
   VkQueue queue;
   VkFence queue_fence;
   struct
   {
      VkCommandPool cmd;
      VkDescriptorPool desc;
   } pools;
   struct
   {
      VkDescriptorSetLayout base;
      VkDescriptorSetLayout renderer;
      VkDescriptorSetLayout texture;
   } set_layouts;
   VkPipelineLayout pipeline_layout;
   struct
   {
      VkSampler nearest;
      VkSampler linear;
   } samplers;
   VkRenderPass renderpass;
} vk_context_t;

void vk_context_init(vk_context_t *vk);
void vk_context_destroy(vk_context_t *vk);
