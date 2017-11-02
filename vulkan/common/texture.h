#pragma once

#include "common.h"
#include "context.h"
#include "memory.h"
#include "buffer.h"
#include "resource.h"

typedef struct
{
   vec2 size;
   int format;
   int ignore_alpha;
} texture_uniform_t;

typedef struct
{
   struct vk_resource_t;
   struct
   {
      device_memory_t mem;
      VkImage image;
      VkFormat format;
      VkImageLayout layout;
   } staging;
   device_memory_t mem;
   VkImage image;
   VkFormat format;
   VkFilter filter;
   VkDescriptorImageInfo info;
   vk_buffer_t ubo;
   texture_uniform_t *uniforms;
   VkDescriptorSet desc;
   int width;
   int height;
   bool ignore_alpha;
   bool is_reference;
} vk_texture_t;

void vk_texture_init(vk_context_t *vk, vk_texture_t *out);
void vk_texture_free(VkDevice device, vk_texture_t *texture);
void vk_texture_update_descriptor_sets(vk_context_t *vk, vk_texture_t *out);
void vk_texture_upload(VkDevice device, VkCommandBuffer cmd, vk_texture_t *texture);

static inline void vk_texture_flush(VkDevice device, VkCommandBuffer cmd, vk_texture_t *texture)
{
   if (!texture->dirty)
      return;

   vk_device_memory_flush(device, &texture->staging.mem);
   vk_texture_upload(device, cmd, texture);

   texture->dirty = false;
}
