
#include "vulkan_common.h"

typedef struct vk_resource_t vk_resource_t;
struct vk_resource_t
{
   bool dirty;
   vk_resource_type_t type;
   vk_resource_t *next;
   vk_resource_t *prev;
};


static vk_resource_t *textures;
static vk_resource_t *buffers;
void vk_resource_add(void *ptr)
{
   vk_resource_t *resource = (vk_resource_t *)ptr;

   vk_resource_t **last;
   if(resource->type == VK_RESOURCE_TEXTURE)
      last = &textures;
   else
      last = &buffers;

   vk_resource_t *prev = NULL;
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
   vk_resource_t* resource = textures;
   while(resource)
   {
      vk_texture_flush(device, cmd, (vk_texture_t*)resource);
      resource = resource->next;
   }

   resource = buffers;
   while(resource)
   {
      vk_buffer_flush(device, (vk_buffer_t*)resource);
      resource = resource->next;
   }
}



