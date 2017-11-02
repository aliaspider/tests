#pragma once

#include "common.h"
#include "memory.h"
#include "resource.h"

typedef struct
{
   struct vk_resource_t;
   VkDescriptorBufferInfo info;
   device_memory_t mem;
   VkBufferUsageFlags usage;
} vk_buffer_t;

void vk_buffer_init(VkDevice device, const VkMemoryType *memory_types, const void *data, vk_buffer_t *out);

static inline void vk_buffer_flush(VkDevice device, vk_buffer_t *buffer)
{
   if (!buffer->dirty)
      return;

   vk_device_memory_flush(device, &buffer->mem);
   buffer->dirty = false;
}

void vk_buffer_invalidate(VkDevice device, vk_buffer_t *buffer);
void vk_buffer_free(VkDevice device, vk_buffer_t *buffer);

