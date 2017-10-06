
#include "vulkan_common.h"

void device_memory_init(VkDevice device, const VkMemoryType* memory_types, const memory_init_info_t* init_info, device_memory_t* dst)
{

   VkMemoryRequirements reqs;

   if(init_info->buffer)
      vkGetBufferMemoryRequirements(device, init_info->buffer, &reqs);
   else
      vkGetImageMemoryRequirements(device, init_info->image, &reqs);

   dst->size = reqs.size;
   dst->alignment = reqs.alignment;

   const VkMemoryType* type = memory_types;
   {
      uint32_t bits = reqs.memoryTypeBits;
      while(bits)
      {
         if((bits & 1) && ((type->propertyFlags & init_info->req_flags) == init_info->req_flags))
            break;

         bits >>= 1;
         type++;
      }
   }

   dst->flags = type->propertyFlags;

   {
      const VkMemoryAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
         .allocationSize = dst->size,
         .memoryTypeIndex = type - memory_types
      };
      vkAllocateMemory(device, &info, NULL, &dst->handle);
   }

   if(init_info->req_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      vkMapMemory(device, dst->handle, 0, dst->size, 0, &dst->ptr);
   else
      dst->ptr = NULL;

   if(init_info->buffer)
      vkBindBufferMemory(device, init_info->buffer, dst->handle, 0);
   else
      vkBindImageMemory(device, init_info->image, dst->handle, 0);
}

void device_memory_free(VkDevice device, device_memory_t* memory)
{
   if(memory->ptr)
      vkUnmapMemory(device, memory->handle);

   vkFreeMemory(device, memory->handle, NULL);

   memory->ptr = NULL;
   memory->flags = 0;
   memory->handle = VK_NULL_HANDLE;
}

void device_memory_flush(VkDevice device, const device_memory_t* memory)
{
   if(memory->flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
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
