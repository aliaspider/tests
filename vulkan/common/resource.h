#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "common.h"

typedef enum
{
   VK_RESOURCE_TEXTURE,
   VK_RESOURCE_BUFFER,
}vk_resource_type_t;

#define VK_RESOURCE \
   bool dirty; \
   vk_resource_type_t type; \
   const uintptr_t private[2]

void vk_resource_add(void* resource);
void vk_resource_remove(void* resource);
void vk_resource_flush_all(VkDevice device, VkCommandBuffer cmd);
