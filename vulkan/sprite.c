
#include <string.h>
#include <math.h>

#include "vulkan_common.h"
#include "common.h"
#include "sprite.h"

typedef union
{
   struct
   {
      int format;
      int ignore_alpha;
   };
   u8 align[VK_UBO_ALIGNMENT];
}uniform_t;

static void vk_sprite_renderer_init(vk_context_t *vk)
{
   const uint32_t vs_code [] =
#include "sprite.vert.inc"
      ;
   const uint32_t ps_code [] =
#include "sprite.frag.inc"
      ;

   const uint32_t gs_code [] =
#include "sprite.geom.inc"
      ;

   const VkVertexInputAttributeDescription attrib_desc[] =
   {
      {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(sprite_t, pos)},
      {1, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(sprite_t, coords)},
      {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(sprite_t, color)},
   };

   const VkPipelineColorBlendAttachmentState color_blend_attachement_state =
   {
      .blendEnable = VK_TRUE,
      .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
      .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .colorBlendOp = VK_BLEND_OP_ADD,
      .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
      .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
      .alphaBlendOp = VK_BLEND_OP_ADD,
      .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
      VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
   };

   const vk_renderer_init_info_t info =
   {
      .shaders.vs.code = vs_code,
      .shaders.vs.code_size = sizeof(vs_code),
      .shaders.ps.code = ps_code,
      .shaders.ps.code_size = sizeof(ps_code),
      .shaders.gs.code = gs_code,
      .shaders.gs.code_size = sizeof(gs_code),
      .attrib_count = countof(attrib_desc),
      .attrib_desc = attrib_desc,
      .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
      .color_blend_attachement_state = &color_blend_attachement_state,
   };

   sprite_renderer.vbo.info.range = VK_RENDERER_MAX_TEXTURES * sizeof(sprite_t);
   sprite_renderer.vertex_stride = sizeof(sprite_t);
   sprite_renderer.ubo.info.range = (1 + VK_RENDERER_MAX_TEXTURES) * sizeof(uniform_t);

   vk_renderer_init(vk, &info, &sprite_renderer);
}

void vk_sprite_add(sprite_t *sprite, vk_texture_t *texture)
{
   int vertex_count = (sprite_renderer.vbo.info.range - sprite_renderer.vbo.info.offset) / sizeof(sprite_t);

   if(texture)
   {
      sprite_renderer.textures[vertex_count] = texture;
      ((uniform_t*)sprite_renderer.ubo.mem.ptr)[vertex_count + 1].format = texture->format;
      ((uniform_t*)sprite_renderer.ubo.mem.ptr)[vertex_count + 1].ignore_alpha = texture->ignore_alpha;
      sprite_renderer.ubo.dirty = true;
   }

   *(sprite_t *)vk_get_vbo_memory(&sprite_renderer.vbo, sizeof(sprite_t)) = *sprite;
}

vk_renderer_t sprite_renderer =
{
   .init = vk_sprite_renderer_init,
   .destroy = vk_renderer_destroy,
   .update = vk_renderer_update,
   .exec = vk_renderer_exec,
   .finish = vk_renderer_finish,
};

