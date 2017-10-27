
#include <string.h>
#include <math.h>

#include "vulkan_common.h"
#include "common.h"
#include "sprite.h"

#define MAX_SPRITES 256
static VkDevice device;
static vk_texture_t *textures[MAX_SPRITES];

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
   };

   const VkPipelineColorBlendAttachmentState color_blend_attachement_state =
   {
      .blendEnable = VK_FALSE,
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

   sprite_renderer.vbo.info.range = MAX_SPRITES * sizeof(sprite_t);
   sprite_renderer.vertex_stride = sizeof(sprite_t);

   vk_renderer_init(vk, &info, &sprite_renderer);
   device = vk->device;
}

void vk_sprite_update(VkDevice device, VkCommandBuffer cmd, vk_renderer_t *renderer)
{
   for (int i = 0; i < (sprite_renderer.vbo.info.range - sprite_renderer.vbo.info.offset) / sizeof(sprite_t); i++)
   {
      if (textures[i]->dirty && !textures[i]->flushed)
         vk_texture_flush(device, textures[i]);

      if (textures[i]->dirty && !textures[i]->uploaded)
         vk_texture_upload(device, cmd, textures[i]);
   }
}

void vk_sprite_add(sprite_t *sprite, vk_texture_t *texture)
{
   textures[(sprite_renderer.vbo.info.range - sprite_renderer.vbo.info.offset) / sizeof(sprite_t)] = texture;
   *(sprite_t *)vk_get_vbo_memory(&sprite_renderer.vbo, sizeof(sprite_t)) = *sprite;
}

void vk_sprite_emit(VkCommandBuffer cmd, vk_renderer_t *renderer)
{
   if (renderer->vbo.info.range - renderer->vbo.info.offset == 0)
      return;

   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipe);
//   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, sprite_renderer.layout, 0, 1, &sprite_renderer.desc, 0, NULL);
   vkCmdBindVertexBuffers(cmd, 0, 1, &renderer->vbo.info.buffer, &renderer->vbo.info.offset);

   int count = (sprite_renderer.vbo.info.range - sprite_renderer.vbo.info.offset) / sizeof(sprite_t);
   int first_vertex = 0;

   for (int i = 0; i < count; i++)
   {
      if (i + 1 < count && textures[i] == textures[i + 1])
         continue;

      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, sprite_renderer.layout, 0, 1, &textures[i]->desc, 0, NULL);
      vkCmdDraw(cmd, i + 1 - first_vertex, 1, first_vertex, 0);
      first_vertex = i + 1;
   }

   renderer->vbo.info.offset = renderer->vbo.info.range;
}

void vk_sprite_finish(VkDevice device, vk_renderer_t *renderer)
{
   if (renderer->vbo.dirty)
      vk_buffer_flush(device, &renderer->vbo);

   renderer->vbo.info.offset = 0;
   renderer->vbo.info.range = 0;
}

vk_renderer_t sprite_renderer =
{
   .init = vk_sprite_renderer_init,
   .destroy = vk_renderer_destroy,
   .update = vk_sprite_update,
   .exec = vk_sprite_emit,
   .finish = vk_sprite_finish,
};

