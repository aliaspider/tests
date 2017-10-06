
#include <string.h>
#include "vulkan_common.h"

void buffer_init(VkDevice device, const VkMemoryType *memory_types, const buffer_init_info_t *init_info, buffer_t *dst)
{
   dst->size = init_info->size;

   {
      const VkBufferCreateInfo info =
      {
         VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
         .size = dst->size,
         .usage = init_info->usage,
      };
      vkCreateBuffer(device, &info, NULL, &dst->handle);
   }

   {
      memory_init_info_t info =
      {
         .req_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
         .buffer = dst->handle
      };
      device_memory_init(device, memory_types, &info, &dst->mem);
   }

   if(init_info->data)
   {
      memcpy(dst->mem.ptr, init_info->data, init_info->size);
      device_memory_flush(device, &dst->mem);
   }
}

void buffer_free(VkDevice device, buffer_t *buffer)
{
   device_memory_free(device, &buffer->mem);
   vkDestroyBuffer(device, buffer->handle, NULL);
   buffer->handle = VK_NULL_HANDLE;
}
