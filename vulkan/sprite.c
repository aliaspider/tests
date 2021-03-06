
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
      {.location = 2, .binding = 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(sprite_t, color)},
      {.location = 3, .binding = 0, VK_FORMAT_R8G8_UNORM, offsetof(sprite_t, effect)},
   };

   static const VkPipelineColorBlendAttachmentState blend_state =
   {
      .blendEnable = VK_TRUE,
      .srcColorBlendFactor = VK_SRC_ALPHA, VK_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_SRC_ALPHA, VK_ONE_MINUS_SRC_ALPHA, VK_BLEND_OP_ADD,
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

   R_sprite.vbo.info.range = 256 * sizeof(sprite_t);
   R_sprite.vertex_stride = sizeof(sprite_t);

   vk_renderer_init(vk, &info, &R_sprite);
}

void vk_sprite_add(sprite_t *sprite, vk_texture_t *texture)
{
   vk_renderer_bind_texture(&R_sprite, texture);
   *(sprite_t *)vk_get_vbo_memory(&R_sprite.vbo, sizeof(sprite_t)) = *sprite;
}

vk_renderer_t R_sprite =
{
   .init = vk_sprite_renderer_init,
   .destroy = vk_renderer_destroy,
   .begin = vk_renderer_begin,
   .finish = vk_renderer_finish,
};

