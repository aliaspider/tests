
#include <string.h>

#include "vulkan_common.h"

void vk_texture_init(VkDevice device, const VkMemoryType *memory_types, uint32_t queue_family_index, vk_texture_t *dst)
{
   dst->dirty = true;
   dst->info.sampler = VK_NULL_HANDLE;

   {
      VkImageCreateInfo info =
      {
         VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
         .flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
         .imageType = VK_IMAGE_TYPE_2D,
         .format = dst->format,
         .extent.width = dst->width,
         .extent.height = dst->height,
         .extent.depth = 1,
         .mipLevels = 1,
         .arrayLayers = 1,
         .samples = VK_SAMPLE_COUNT_1_BIT,
         .tiling = VK_IMAGE_TILING_OPTIMAL,
         .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
         .queueFamilyIndexCount = 1,
         .pQueueFamilyIndices = &queue_family_index,
         .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
      };

      dst->info.imageLayout = info.initialLayout;
      vkCreateImage(device, &info, NULL, &dst->image);

      info.tiling = VK_IMAGE_TILING_LINEAR;
      info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
      info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
      dst->staging.format = info.format;
      dst->staging.layout = info.initialLayout;
      vkCreateImage(device, &info, NULL, &dst->staging.image);
   }

   {
      VkImageSubresource imageSubresource =
      {
         .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .mipLevel = 0,
         .arrayLayer = 0
      };
//      vkGetImageSubresourceLayout(device, dst->image, &imageSubresource, &dst->mem.layout);
      vkGetImageSubresourceLayout(device, dst->staging.image, &imageSubresource, &dst->staging.mem.layout);
   }

   {
      memory_init_info_t info =
      {
         .req_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         .image = dst->image
      };
      device_memory_init(device, memory_types, &info, &dst->mem);
   }
   {
      memory_init_info_t info =
      {
         .req_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
         .image = dst->staging.image
      };
      device_memory_init(device, memory_types, &info, &dst->staging.mem);
   }

   {
      VkImageViewCreateInfo info =
      {
         VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
         .image = dst->image,
         .viewType = VK_IMAGE_VIEW_TYPE_2D,
         .format = dst->format,
         .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .subresourceRange.levelCount = 1,
         .subresourceRange.layerCount = 1
      };
      vkCreateImageView(device, &info, NULL, &dst->info.imageView);
   }
}

void texture_free(VkDevice device, vk_texture_t *texture)
{
   vkDestroyImageView(device, texture->info.imageView, NULL);
   vkDestroyImage(device, texture->image, NULL);
   vkDestroyImage(device, texture->staging.image, NULL);
   device_memory_free(device, &texture->mem);
   device_memory_free(device, &texture->staging.mem);
   texture->info.imageView = VK_NULL_HANDLE;
   texture->image = VK_NULL_HANDLE;
}

void texture_update(VkDevice device, VkCommandBuffer cmd, vk_texture_t *texture)
{
   device_memory_flush(device, &texture->staging.mem);

   VkImageMemoryBarrier barrier =
   {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcAccessMask = VK_ACCESS_HOST_WRITE_BIT,
      .dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT,
      .oldLayout = texture->staging.layout,
      .newLayout = VK_IMAGE_LAYOUT_GENERAL,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .image  = texture->staging.image,
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.levelCount = 1,
      .subresourceRange.layerCount = 1
   };
   texture->staging.layout = barrier.newLayout;
   vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1, &barrier);

   barrier.srcAccessMask = 0;
   barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
   barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
   barrier.image  = texture->image;
   texture->info.imageLayout = barrier.newLayout;
   vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT, 0, 0, NULL, 0, NULL, 1,
      &barrier);

   if (texture->format == texture->staging.format)
   {
      const VkImageCopy copy =
      {
         .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .srcSubresource.layerCount = 1,
         .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .dstSubresource.layerCount = 1,
         .extent.width = texture->width,
         .extent.height = texture->height,
         .extent.depth = 1
      };
      vkCmdCopyImage(cmd, texture->staging.image, texture->staging.layout, texture->image, texture->info.imageLayout, 1,
         &copy);
   }
   else
   {
      const VkImageBlit blit =
      {
         .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .srcSubresource.layerCount = 1,
         .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .dstSubresource.layerCount = 1,
         .srcOffsets = {{0, 0, 0}, {texture->width, texture->height, 1}},
         .dstOffsets = {{0, 0, 0}, {texture->width, texture->height, 1}}
      };
      vkCmdBlitImage(cmd, texture->staging.image, texture->staging.layout, texture->image, texture->info.imageLayout, 1,
         &blit, VK_FILTER_NEAREST);
   }

   barrier.srcAccessMask = barrier.dstAccessMask;
   barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
   barrier.oldLayout = barrier.newLayout;
   barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   texture->info.imageLayout = barrier.newLayout;
   vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT, 0, 0, NULL, 0, NULL, 1,
      &barrier);

   texture->dirty = false;
}


void device_memory_init(VkDevice device, const VkMemoryType *memory_types, const memory_init_info_t *init_info,
   device_memory_t *dst)
{

   VkMemoryRequirements reqs;

   if (init_info->buffer)
      vkGetBufferMemoryRequirements(device, init_info->buffer, &reqs);
   else
      vkGetImageMemoryRequirements(device, init_info->image, &reqs);

   dst->size = reqs.size;
   dst->alignment = reqs.alignment;

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

   if (init_info->req_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      vkMapMemory(device, dst->handle, 0, dst->size, 0, &dst->ptr);
   else
      dst->ptr = NULL;

   if (init_info->buffer)
      vkBindBufferMemory(device, init_info->buffer, dst->handle, 0);
   else
      vkBindImageMemory(device, init_info->image, dst->handle, 0);
}

void device_memory_free(VkDevice device, device_memory_t *memory)
{
   if (memory->ptr)
      vkUnmapMemory(device, memory->handle);

   vkFreeMemory(device, memory->handle, NULL);

   memory->ptr = NULL;
   memory->flags = 0;
   memory->handle = VK_NULL_HANDLE;
}

void device_memory_flush(VkDevice device, const device_memory_t *memory)
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

void vk_buffer_init(VkDevice device, const VkMemoryType *memory_types, const void *data, vk_buffer_t *dst)
{
   {
      const VkBufferCreateInfo info =
      {
         VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
         .size = dst->info.range,
         .usage = dst->usage,
      };
      vkCreateBuffer(device, &info, NULL, &dst->info.buffer);
   }

   {
      memory_init_info_t info =
      {
         .req_flags = dst->mem.flags,
         .buffer = dst->info.buffer
      };
      device_memory_init(device, memory_types, &info, &dst->mem);
   }

   if (data && dst->mem.ptr)
   {
      memcpy(dst->mem.ptr, data, dst->info.range);
      device_memory_flush(device, &dst->mem);
   }
}

void buffer_free(VkDevice device, vk_buffer_t *buffer)
{
   device_memory_free(device, &buffer->mem);
   vkDestroyBuffer(device, buffer->info.buffer, NULL);
   buffer->info.buffer = VK_NULL_HANDLE;
}

void buffer_flush(VkDevice device, vk_buffer_t *buffer)
{
   device_memory_flush(device, &buffer->mem);
   buffer->dirty = false;
}

void vk_get_instance_props(void)
{
   uint32_t lprop_count;
   vkEnumerateInstanceLayerProperties(&lprop_count, NULL);
   VkLayerProperties lprops[lprop_count + 1];
   vkEnumerateInstanceLayerProperties(&lprop_count, lprops);
   lprops[lprop_count].layerName[0] = '\0';

   int l;

   for (l = 0; l < lprop_count + 1; l++)
   {
      uint32_t iexprop_count;
      vkEnumerateInstanceExtensionProperties(lprops[l].layerName, &iexprop_count, NULL);
      VkExtensionProperties iexprops[iexprop_count];
      vkEnumerateInstanceExtensionProperties(lprops[l].layerName, &iexprop_count, iexprops);
      printf("%s (%i)\n", lprops[l].layerName, iexprop_count);

      int e;

      for (e = 0; e < iexprop_count; e++)
         printf("\t%s\n", iexprops[e].extensionName);
   }

   fflush(stdout);
}

void vk_get_gpu_props(VkPhysicalDevice gpu)
{
   VkPhysicalDeviceProperties gpu_props;
   vkGetPhysicalDeviceProperties(gpu, &gpu_props);

   uint32_t deviceExtensionPropertiesCount;
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &deviceExtensionPropertiesCount, NULL);

   VkExtensionProperties pDeviceExtensionProperties[deviceExtensionPropertiesCount];
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &deviceExtensionPropertiesCount,
      pDeviceExtensionProperties);

   int e;

   for (e = 0; e < deviceExtensionPropertiesCount; e++)
      printf("\t%s\n", pDeviceExtensionProperties[e].extensionName);

#if 0
   {
      uint32_t displayProperties_count;
      vkGetPhysicalDeviceDisplayPropertiesKHR(vk.gpu, &displayProperties_count, NULL);
      VkDisplayPropertiesKHR displayProperties[displayProperties_count];
      vkGetPhysicalDeviceDisplayPropertiesKHR(vk.gpu, &displayProperties_count, displayProperties);
      dst->display = displayProperties[0].display;
      int i;

      for (i = 0; i < displayProperties_count; i++)
         printf("0x%08" PRIXPTR " : %s\n", (uintptr_t)displayProperties[i].display,
            displayProperties[i].displayName);

   }
   vkGetPhysicalDeviceDisplayPlanePropertiesKHR(vk.gpu, &displayProperties_count, NULL);
   VkDisplayPlanePropertiesKHR displayPlaneProperties[displayProperties_count];
   vkGetPhysicalDeviceDisplayPlanePropertiesKHR(vk.gpu, &displayProperties_count, displayPlaneProperties);

   for (i = 0; i < displayProperties_count; i++)
      printf("0x%08" PRIXPTR " : %i\n", (uintptr_t)displayPlaneProperties[i].currentDisplay,
         displayPlaneProperties[i].currentStackIndex);

//   uint32_t displayCount = 4;
//   VK_CHECK(vkGetDisplayPlaneSupportedDisplaysKHR(vk.gpu, 1, &displayCount, &dst->display));


//   VK_CHECK(vkGetRandROutputDisplayEXT(vk.gpu, init_info->display, 0x27e, &dst->display));
//   VK_CHECK(vkAcquireXlibDisplayEXT(vk.gpu, init_info->display, dst->display));
//   VK_CHECK(vkReleaseDisplayEXT(vk.gpu, dst->display));
//   exit(0);


   uint32_t                   displayModeCount;
   VK_CHECK(vkGetDisplayModePropertiesKHR(vk.gpu, dst->display, &displayModeCount, NULL));
   VkDisplayModePropertiesKHR displayModeProperties[displayModeCount];
   VK_CHECK(vkGetDisplayModePropertiesKHR(vk.gpu, dst->display, &displayModeCount, displayModeProperties));
   printf("displayModeProperties.parameters.refreshRate : %u\n", displayModeProperties[0].parameters.refreshRate);
   printf("displayModeProperties.visibleRegion.width : %u\n", displayModeProperties[0].parameters.visibleRegion.width);
   printf("displayModeProperties.visibleRegion.height : %u\n", displayModeProperties[0].parameters.visibleRegion.height);
#endif


}

void vk_get_surface_props(VkPhysicalDevice gpu, uint32_t queue_family_index, VkSurfaceKHR surface)
{
   {
      VkSurfaceCapabilitiesKHR surfaceCapabilities;
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCapabilities);
#if 0
      VkSurfaceCapabilities2EXT surfaceCapabilities2 = {VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT};
      vkGetPhysicalDeviceSurfaceCapabilities2EXT(gpu, surface, &surfaceCapabilities2);
      printf("surfaceCapabilities2.supportedSurfaceCounters : %i\n", surfaceCapabilities2.supportedSurfaceCounters);
#endif
      VkBool32 physicalDeviceSurfaceSupport;
      vkGetPhysicalDeviceSurfaceSupportKHR(gpu, queue_family_index, surface, &physicalDeviceSurfaceSupport);

      uint32_t surfaceFormatcount;
      vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surfaceFormatcount, NULL);
      VkSurfaceFormatKHR surfaceFormats[surfaceFormatcount];
      vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surfaceFormatcount, surfaceFormats);

      uint32_t presentModeCount;
      vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, NULL);
      VkPresentModeKHR presentModes[presentModeCount];
      vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, presentModes);
      int i;

      for (i = 0; i < presentModeCount; i++)
         printf("supports present mode %i\n", presentModes[i]);
   }
}

uint32_t vk_get_queue_family_index(VkPhysicalDevice gpu, VkQueueFlags required_flags)
{
   uint32_t queueFamilyPropertyCount;
   vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertyCount, NULL);

   VkQueueFamilyProperties pQueueFamilyProperties[queueFamilyPropertyCount];
   vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertyCount, pQueueFamilyProperties);

   int i;

   for (i = 0; i < queueFamilyPropertyCount; i++)
      if ((pQueueFamilyProperties[i].queueFlags & required_flags) == required_flags)
         return i;

   return 0;
}


static inline VkShaderModule vk_shader_code_init(VkDevice device, const vk_shader_code_t *shader_code)
{
   VkShaderModule shader;

   const VkShaderModuleCreateInfo info =
   {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = shader_code->code_size,
      .pCode = shader_code->code
   };
   vkCreateShaderModule(device, &info, NULL, &shader);

   return shader;
}

void vk_pipeline_init(vk_context_t *vk, const vk_pipeline_init_info_t *init_info, vk_pipeline_t *dst)
{
   if (dst->texture.image)
      dst->texture.is_reference = true;
   else
      vk_texture_init(vk->device, vk->memoryTypes, vk->queue_family_index, &dst->texture);

   if (dst->ssbo.info.range)
   {
      dst->ssbo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      dst->ssbo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
      vk_buffer_init(vk->device, vk->memoryTypes, NULL, &dst->ssbo);
   }

   if (dst->ubo.info.range)
   {
      dst->ubo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      dst->ubo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      vk_buffer_init(vk->device, vk->memoryTypes, NULL, &dst->ubo);
   }

   dst->vbo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   dst->vbo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
   vk_buffer_init(vk->device, vk->memoryTypes, NULL, &dst->vbo);

   {
      const VkDescriptorSetAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .descriptorPool = vk->pools.desc,
         .descriptorSetCount = 1, &vk->descriptor_set_layout
      };
      vkAllocateDescriptorSets(vk->device, &info, &dst->desc);
   }

   {
      VkWriteDescriptorSet write_set[3];
      int write_set_count = 0;

      if (dst->ubo.info.buffer)
      {
         VkWriteDescriptorSet set =
         {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = dst->desc,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &dst->ubo.info
         };
         write_set[write_set_count++] = set;
      }

      VkDescriptorImageInfo image_info[] =
      {
         {
            .sampler = dst->texture.info.sampler,
            .imageView = dst->texture.info.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
         },
         {
            .sampler = dst->texture.info.sampler,
            .imageView = dst->texture.info.imageView,
            .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
         }
      };

      if (dst->texture.image)
      {
         const VkWriteDescriptorSet set =
         {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = dst->desc,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = countof(image_info),
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = image_info
         };
         write_set[write_set_count++] = set;
      }

      if (dst->ssbo.info.buffer)
      {
         VkWriteDescriptorSet set =
         {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = dst->desc,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pBufferInfo = &dst->ssbo.info
         };
         write_set[write_set_count++] = set;
      }

      vkUpdateDescriptorSets(vk->device, write_set_count, write_set, 0, NULL);
   }


   dst->layout = vk->pipeline_layout;

   {
      const VkPipelineShaderStageCreateInfo shaders_info[] =
      {
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .pName = "main",
            .module = vk_shader_code_init(vk->device, &init_info->shaders.vs)
         },
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pName = "main",
            .module = vk_shader_code_init(vk->device, &init_info->shaders.ps)
         },
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
            .pName = "main",
            .module = init_info->shaders.gs.code ? vk_shader_code_init(vk->device, &init_info->shaders.gs) : VK_NULL_HANDLE
         }
      };

      const VkVertexInputBindingDescription vertex_description =
      {
         0, init_info->vertex_stride, VK_VERTEX_INPUT_RATE_VERTEX
      };

      const VkPipelineVertexInputStateCreateInfo vertex_input_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
         .vertexBindingDescriptionCount = 1, &vertex_description,
         .vertexAttributeDescriptionCount = init_info->attrib_count, init_info->attrib_desc
      };

      const VkPipelineInputAssemblyStateCreateInfo input_assembly_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
         .topology = init_info->topology,
         .primitiveRestartEnable = VK_FALSE
      };

      VkPipelineViewportStateCreateInfo viewport_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
         .viewportCount = 1, .scissorCount = 1
      };

      const VkPipelineRasterizationStateCreateInfo rasterization_info =
      {
         VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
//         .rasterizerDiscardEnable = VK_TRUE,
         .lineWidth = 1.0f
      };

      const VkPipelineMultisampleStateCreateInfo multisample_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
         .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      };

      const VkPipelineColorBlendStateCreateInfo colorblend_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
         .attachmentCount = 1, init_info->color_blend_attachement_state
      };

      const VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
      const VkPipelineDynamicStateCreateInfo dynamic_state_info =
      {
         VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
         .dynamicStateCount = countof(dynamic_states), dynamic_states
      };

      VkGraphicsPipelineCreateInfo info[MAX_SCREENS] =
      {
         {
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT | VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
            .stageCount = shaders_info[2].module ? 3 : 2, shaders_info,
            .pVertexInputState = &vertex_input_state,
            .pInputAssemblyState = &input_assembly_state,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterization_info,
            .pMultisampleState = &multisample_state,
            .pColorBlendState = &colorblend_state,
            .pDynamicState = &dynamic_state_info,
            .layout = dst->layout,
            .renderPass = vk->renderpass,
            .subpass = 0
         }
      };
      vkCreateGraphicsPipelines(vk->device, VK_NULL_HANDLE, 1, info, NULL, &dst->handle);

      vkDestroyShaderModule(vk->device, shaders_info[0].module, NULL);
      vkDestroyShaderModule(vk->device, shaders_info[1].module, NULL);

      if (shaders_info[2].module)
         vkDestroyShaderModule(vk->device, shaders_info[2].module, NULL);
   }

}

void vk_pipeline_destroy(VkDevice device, vk_pipeline_t *render)
{
   vkDestroyPipeline(device, render->handle, NULL);
   buffer_free(device, &render->vbo);
   buffer_free(device, &render->ubo);
   buffer_free(device, &render->ssbo);
   texture_free(device, &render->texture);

   memset(render, 0, sizeof(*render));
}
