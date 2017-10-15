
#include <stdlib.h>
#include <string.h>
#include "vulkan_common.h"

void descriptors_init(VkDevice device, vk_descriptor_t* dst)
{
   {
      const VkDescriptorPoolSize sizes[] =
      {
         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 1},
         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 1}
      };

      const VkDescriptorPoolCreateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
         .maxSets = 1,
         .poolSizeCount = countof(sizes), sizes
      };
      vkCreateDescriptorPool(device, &info, NULL, &dst->pool);
   }

   {
      const VkDescriptorSetLayoutBinding bindings[] =
      {
         {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
         },
         {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
   //            .pImmutableSamplers = &vk.texture_sampler
         }
      };

      const VkDescriptorSetLayoutCreateInfo info [] =
      {
         {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = countof(bindings), bindings

         }
      };
      vkCreateDescriptorSetLayout(device, &info[0], NULL, &dst->set_layout);
   }

}

void descriptors_update(VkDevice device, const descriptors_update_info_t *update_info, VkDescriptorSet set)
{
   {
//      const VkDescriptorBufferInfo buffer_info =
//      {
//         .buffer = init_info->ubo_buffer,
//         .offset = 0,
//         .range = init_info->ubo_range
//      };
      const VkDescriptorImageInfo image_info =
      {
         .sampler = update_info->sampler,
         .imageView = update_info->image_view,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      };

      const VkWriteDescriptorSet write_set[] =
      {
//         {
//            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
//            .dstSet = dst->set,
//            .dstBinding = 0,
//            .dstArrayElement = 0,
//            .descriptorCount = 1,
//            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
//            .pBufferInfo = &buffer_info
//         },
         {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = set,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &image_info
         }
      };
      vkUpdateDescriptorSets(device, countof(write_set), write_set, 0, NULL);
   }
}

void descriptors_free(VkDevice device, vk_descriptor_t *descriptor)
{
   vkDestroyDescriptorPool(device, descriptor->pool, NULL);
   vkDestroyDescriptorSetLayout(device, descriptor->set_layout, NULL);
   descriptor->pool = VK_NULL_HANDLE;
   descriptor->set_layout = VK_NULL_HANDLE;
}
