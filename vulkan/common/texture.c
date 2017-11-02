
#include "texture.h"

void vk_texture_update_descriptor_sets(vk_context_t *vk, vk_texture_t *out)

{
   const VkWriteDescriptorSet write_set[] =
   {
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = out->desc,
         .dstBinding = 0,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .pImageInfo = &out->info
      },
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = out->desc,
         .dstBinding = 1,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .pBufferInfo = &out->ubo.info,
      }
   };

   if(out->info.imageView)
      vkUpdateDescriptorSets(vk->device, countof(write_set), write_set, 0, NULL);
   else
      vkUpdateDescriptorSets(vk->device, 1, &write_set[1], 0, NULL);
}

void vk_texture_init(vk_context_t *vk, vk_texture_t *out)
{
   if(out->format != VK_FORMAT_UNDEFINED)
   {
      out->info.sampler = out->filter == VK_FILTER_LINEAR ? vk->samplers.linear : vk->samplers.nearest;
      out->info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
      out->staging.format = out->format;

      {
         VkImageCreateInfo info =
         {
            VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
            .flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
            .imageType = VK_IMAGE_TYPE_2D,
            .format = out->format,
            .extent.width = out->width,
            .extent.height = out->height,
            .extent.depth = 1,
            .mipLevels = 1,
            .arrayLayers = 1,
            .samples = VK_SAMPLE_COUNT_1_BIT,
            .tiling = VK_IMAGE_TILING_OPTIMAL,
            .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
            .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
            .queueFamilyIndexCount = 1,
            .pQueueFamilyIndices = &vk->queue_family_index,
            .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
         };

         VK_CHECK(vkCreateImage(vk->device, &info, NULL, &out->image));

         info.tiling = VK_IMAGE_TILING_LINEAR;
         info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
         info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
         out->staging.layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
         VK_CHECK(vkCreateImage(vk->device, &info, NULL, &out->staging.image));
      }

      {
         static const VkImageSubresource imageSubresource =
         {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .arrayLayer = 0
         };
         vkGetImageSubresourceLayout(vk->device, out->staging.image, &imageSubresource, &out->staging.mem.layout);
      }

      {
         memory_init_info_t info =
         {
            .req_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
            .image = out->image
         };
         vk_device_memory_init(vk->device, vk->memoryTypes, &info, &out->mem);
      }
      {
         memory_init_info_t info =
         {
            .req_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
            .image = out->staging.image
         };
         vk_device_memory_init(vk->device, vk->memoryTypes, &info, &out->staging.mem);
      }

      {
         VkImageViewCreateInfo info =
         {
            VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = out->image,
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = out->format,
            .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .subresourceRange.levelCount = 1,
            .subresourceRange.layerCount = 1
         };
         VK_CHECK(vkCreateImageView(vk->device, &info, NULL, &out->info.imageView));
      }
   }
   {
      const VkDescriptorSetAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .descriptorPool = vk->pools.desc,
         .descriptorSetCount = 1, &vk->set_layouts.texture
      };
      VK_CHECK(vkAllocateDescriptorSets(vk->device, &info, &out->desc));
   }

   out->ubo.info.range = 256;
   out->ubo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   out->ubo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
   vk_buffer_init(vk->device, vk->memoryTypes, NULL, &out->ubo);

   vk_texture_update_descriptor_sets(vk, out);

   out->uniforms = out->ubo.mem.ptr;
   out->uniforms->size.width = out->width;
   out->uniforms->size.height = out->height;
   out->uniforms->format = out->format;
   out->uniforms->ignore_alpha = out->ignore_alpha;
   out->ubo.dirty = true;


   out->type = VK_RESOURCE_TEXTURE;
   vk_resource_add(out);
}

void vk_texture_free(VkDevice device, vk_texture_t *texture)
{
   vkDestroyImageView(device, texture->info.imageView, NULL);
   vkDestroyImage(device, texture->image, NULL);
   vkDestroyImage(device, texture->staging.image, NULL);
   vk_device_memory_free(device, &texture->mem);
   vk_device_memory_free(device, &texture->staging.mem);
   vk_buffer_free(device, &texture->ubo);
   texture->info.imageView = VK_NULL_HANDLE;
   texture->image = VK_NULL_HANDLE;
   vk_resource_remove(texture);
}


void vk_texture_upload(VkDevice device, VkCommandBuffer cmd, vk_texture_t *texture)
{
   VkImageMemoryBarrier barrier =
   {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.levelCount = 1,
      .subresourceRange.layerCount = 1
   };

   if (texture->staging.layout == VK_IMAGE_LAYOUT_PREINITIALIZED)
   {
      barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
      barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
      barrier.image  = texture->staging.image;
      vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           0, 0, NULL, 0, NULL, 1, &barrier);

      texture->staging.layout = VK_IMAGE_LAYOUT_GENERAL;
   }

   barrier.srcAccessMask = 0;
   barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
   barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
   barrier.image  = texture->image;
   vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0, 0, NULL, 0, NULL, 1, &barrier);

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
      vkCmdCopyImage(cmd, texture->staging.image, VK_IMAGE_LAYOUT_GENERAL,
                     texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     1, &copy);
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
      vkCmdBlitImage(cmd, texture->staging.image, VK_IMAGE_LAYOUT_GENERAL,
                     texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     1, &blit, VK_FILTER_NEAREST);
   }

   barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
   barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
   barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
   barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        0, 0, NULL, 0, NULL, 1, &barrier);
}
