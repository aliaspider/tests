
#include <string.h>
#include <math.h>

#include "vulkan_common.h"
#include "common.h"
#include "sprite.h"

static void vk_sprite_renderer_init(vk_context_t *vk)
{

#define SHADER_FILE sprite
#include "shaders.h"

   static const VkVertexInputAttributeDescription attrib_desc[] =
   {
      {.location = 0, .binding = 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(sprite_t, pos)},
      {.location = 1, .binding = 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(sprite_t, coords)},
      {.location = 2, .binding = 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(sprite_t, color)},
   };

   static const VkPipelineColorBlendAttachmentState blend_state =
   {
      .blendEnable = VK_TRUE,
      .srcColorBlendFactor = VK_SRC_ALPHA, VK_ONE_MINUS_SRC_ALPHA, VK_ADD,
      .srcAlphaBlendFactor = VK_SRC_ALPHA, VK_ONE_MINUS_SRC_ALPHA, VK_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_ALL
   };

   static const vk_renderer_init_info_t info =
   {
      SHADER_INFO,
      .attrib_count = countof(attrib_desc),
      .attrib_desc = attrib_desc,
      .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
      .color_blend_attachement_state = &blend_state,
   };

   R_sprite.vbo.info.range = VK_RENDERER_MAX_TEXTURES * sizeof(sprite_t);
   R_sprite.vertex_stride = sizeof(sprite_t);

   vk_renderer_init(vk, &info, &R_sprite);
}

void vk_sprite_add(sprite_t *sprite, vk_texture_t *texture)
{
   if(!texture)
      texture = &R_sprite.tex;

   if(R_sprite.desc.texture != texture->desc)
   {
      if (R_sprite.vbo.info.range)
      {
         int count = R_sprite.vbo.info.range / R_sprite.vertex_stride;

         vkCmdDraw(R_sprite.cmd, count, 1, R_sprite.first_vertex, 0);

         R_sprite.first_vertex += count;
         R_sprite.vbo.info.offset += R_sprite.vbo.info.range;
         R_sprite.vbo.info.range = 0;
      }

      R_sprite.desc.texture = texture->desc;

      vkCmdBindDescriptorSets(R_sprite.cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, R_sprite.pipeline_layout, 2, 1, &R_sprite.desc.texture, 0, NULL);
   }

   *(sprite_t *)vk_get_vbo_memory(&R_sprite.vbo, sizeof(sprite_t)) = *sprite;
}

vk_renderer_t R_sprite =
{
   .init = vk_sprite_renderer_init,
   .destroy = vk_renderer_destroy,
   .begin = vk_renderer_begin,
   .finish = vk_renderer_finish,
};

