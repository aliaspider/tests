
#include <string.h>

#include "buffer.h"
#include "memory.h"

void vk_buffer_init(VkDevice device, const VkMemoryType *memory_types, const void *data, vk_buffer_t *out)
{
   {
      const VkBufferCreateInfo info =
      {
         VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
         .size = out->info.range,
         .usage = out->usage,
      };
      vkCreateBuffer(device, &info, NULL, &out->info.buffer);
   }

   {
      memory_init_info_t info =
      {
         .req_flags = out->mem.flags,
         .buffer = out->info.buffer
      };
      vk_device_memory_init(device, memory_types, &info, &out->mem);
   }

   if (data && out->mem.ptr)
   {
      memcpy(out->mem.ptr, data, out->info.range);
      vk_device_memory_flush(device, &out->mem);
   }

   out->type = VK_RESOURCE_BUFFER;
   vk_resource_add(out);
}

void vk_buffer_free(VkDevice device, vk_buffer_t *buffer)
{
   vk_device_memory_free(device, &buffer->mem);
   vkDestroyBuffer(device, buffer->info.buffer, NULL);
   buffer->info.buffer = VK_NULL_HANDLE;
   vk_resource_remove(buffer);
}

void vk_buffer_invalidate(VkDevice device, vk_buffer_t *buffer)
{
   device_memory_invalidate(device, &buffer->mem);
   buffer->dirty = false;
}
