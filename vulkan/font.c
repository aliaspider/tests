
#include "vulkan_common.h"

#define VK_ATLAS_WIDTH  256
#define VK_ATLAS_HEIGHT 256
vk_texture_t atlas;
VkDescriptorSet atlas_desc;

void vulkan_font_init(VkDevice device, uint32_t queue_family_index, const VkMemoryType *memory_types, vk_descriptor_t* desc)
{
      {
         texture_init_info_t info =
         {
            .queue_family_index = queue_family_index,
            .width = VK_ATLAS_WIDTH,
            .height = VK_ATLAS_HEIGHT,
            .format = VK_FORMAT_R8_UNORM,
            .filter = VK_FILTER_NEAREST
         };
         texture_init(device, memory_types, &info, &atlas);
      }

      memset(atlas.staging.mem.u8 + atlas.staging.mem_layout.offset, 0xFF,
             atlas.staging.mem_layout.size - atlas.staging.mem_layout.offset);

      device_memory_flush(device, &atlas.staging.mem);
      atlas.dirty = true;

      {
         const VkDescriptorSetAllocateInfo info =
         {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
            .descriptorPool = desc->pool,
            .descriptorSetCount = 1, &desc->set_layout
         };
         vkAllocateDescriptorSets(device, &info, &atlas_desc);
      }

      {
         descriptors_update_info_t info =
         {
//         .ubo_buffer = ubo.handle,
//         .ubo_range = ubo.size,
            .sampler = atlas.sampler,
            .image_view = atlas.view,
         };
         descriptors_update(device, &info, atlas_desc);
      }

}

void vulkan_font_destroy()
{

}
