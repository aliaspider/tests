#define RESOURCE_INTERNAL

#include "resource.h"
#include "buffer.h"
#include "texture.h"

static vk_resource_t *textures;
static vk_resource_t *buffers;

void vk_resource_add(void *ptr)
{
	vk_resource_t * resource = (vk_resource_t *)ptr;

	if(resource->type == VK_RESOURCE_BUFFER && (((vk_buffer_t*)ptr)->mem.flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		return;

	vk_resource_t **last;
	if(resource->type == VK_RESOURCE_TEXTURE)
		last = &textures;
	else
		last = &buffers;

	vk_resource_t * prev = NULL;
	while (*last)
	{
		prev = *last;
		last = &(*last)->next;
	}

	*last = resource;

	resource->prev = prev;
	resource->next = NULL;
}

void vk_resource_remove(void *ptr)
{
	vk_resource_t *resource = (vk_resource_t *)ptr;

	if(resource->type == VK_RESOURCE_BUFFER && (((vk_buffer_t*)ptr)->mem.flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT))
		return;

	if (resource->next)
		resource->next->prev = resource->prev;

	if (resource->prev)
		resource->prev->next = resource->next;
	else
	{
		if(resource->type == VK_RESOURCE_TEXTURE)
			textures = resource->next;
		else
			buffers = resource->next;
	}

}

void vk_resource_flush_all(VkDevice device, VkCommandBuffer cmd)
{
	for(vk_resource_t* resource = textures; resource; resource = resource->next)
		vk_texture_flush(device, cmd, (vk_texture_t*)resource);

	for(vk_resource_t* resource = buffers; resource; resource = resource->next)
		vk_buffer_flush(device, (vk_buffer_t*)resource);
}
