
#include "memory.h"

void vk_device_memory_init(VkDevice device, const VkMemoryType *memory_types, const memory_init_info_t *init_info,
                           device_memory_t *out)
{

   VkMemoryRequirements reqs;

   if (init_info->buffer)
      vkGetBufferMemoryRequirements(device, init_info->buffer, &reqs);
   else
      vkGetImageMemoryRequirements(device, init_info->image, &reqs);

   out->size = reqs.size;
   out->alignment = reqs.alignment;

   const VkMemoryType *type = memory_types;
   {
      uint32_t bits = reqs.memoryTypeBits;

      while (bits)
      {
         if ((bits & 1) && ((type->propertyFlags & init_info->req_flags) == init_info->req_flags))
            break;

         bits >>= 1;
         type++;
      }
   }

   out->flags = type->propertyFlags;

   {
      const VkMemoryAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
         .allocationSize = out->size,
         .memoryTypeIndex = type - memory_types
      };
      VK_CHECK(vkAllocateMemory(device, &info, NULL, &out->handle));
   }

   if (init_info->req_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      vkMapMemory(device, out->handle, 0, out->size, 0, &out->ptr);
   else
      out->ptr = NULL;

   if (init_info->buffer)
      vkBindBufferMemory(device, init_info->buffer, out->handle, 0);
   else
      vkBindImageMemory(device, init_info->image, out->handle, 0);
}

void vk_device_memory_free(VkDevice device, device_memory_t *memory)
{
   if (memory->ptr)
      vkUnmapMemory(device, memory->handle);

   vkFreeMemory(device, memory->handle, NULL);

   memory->ptr = NULL;
   memory->flags = 0;
   memory->handle = VK_NULL_HANDLE;
}

void vk_device_memory_flush(VkDevice device, const device_memory_t *memory)
{
   if (memory->flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
      return;

   {
      VkMappedMemoryRange range =
      {
         VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
         .memory = memory->handle,
         .offset = 0,
         .size = memory->size
      };
      vkFlushMappedMemoryRanges(device, 1, &range);
   }
}

void device_memory_invalidate(VkDevice device, const device_memory_t *memory)
{
   if (memory->flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
      return;

   {
      VkMappedMemoryRange range =
      {
         VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
         .memory = memory->handle,
         .offset = 0,
         .size = memory->size
      };
      vkInvalidateMappedMemoryRanges(device, 1, &range);
   }
}
