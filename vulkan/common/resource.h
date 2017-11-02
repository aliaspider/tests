#pragma once

#include <stdint.h>
#include <stdbool.h>
#include "common.h"

typedef enum
{
   VK_RESOURCE_TEXTURE,
   VK_RESOURCE_BUFFER,
}vk_resource_type_t;

typedef struct vk_resource_t vk_resource_t;
struct vk_resource_t
{
	bool dirty;
	vk_resource_type_t type;
#ifdef RESOURCE_INTERNAL
	vk_resource_t *next;
	vk_resource_t *prev;
#else
   const void* private[2];
#endif
};

void vk_resource_add(void* resource);
void vk_resource_remove(void* resource);
void vk_resource_flush_all(VkDevice device, VkCommandBuffer cmd);
