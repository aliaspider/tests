#pragma once

#include "common.h"


typedef struct
{
   VkDeviceMemory handle;
   VkMemoryPropertyFlags flags;
   VkDeviceSize size;
   VkDeviceSize alignment;
   VkSubresourceLayout layout;
   union
   {
      void *ptr;
      uint8_t *u8;
   };
} device_memory_t;

typedef struct
{
   VkMemoryPropertyFlags req_flags;
   VkBuffer buffer;
   VkImage image;
} memory_init_info_t;
void vk_device_memory_init(VkDevice device, const VkMemoryType *memory_types, const memory_init_info_t *init_info,
                           device_memory_t *out);
void vk_device_memory_free(VkDevice device, device_memory_t *memory);
void vk_device_memory_flush(VkDevice device, const device_memory_t *memory);
void device_memory_invalidate(VkDevice device, const device_memory_t *memory);
