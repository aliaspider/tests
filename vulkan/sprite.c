
#include <string.h>
#include <math.h>

#include "vulkan_common.h"
#include "common.h"
#include "sprite.h"

static int current_cmd_id;
static VkCommandBuffer current_main_cmd;
static VkDevice device;
static VkRenderPass renderpass;

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
      {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(sprite_t, tex_size)},
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

   sprite_renderer.vbo.info.range = 256 * sizeof(sprite_t);
   sprite_renderer.vertex_stride = sizeof(sprite_t);

   vk_renderer_init(vk, &info, &sprite_renderer);
   device = vk->device;
   renderpass = vk->renderpass;
}

void vk_sprite_update(VkDevice device, VkCommandBuffer main_cmd, vk_renderer_t *renderer)
{
   current_main_cmd = main_cmd;

   VkCommandBuffer cmd = renderer->cmd[current_cmd_id];
   {
      const VkCommandBufferInheritanceInfo inheritanceInfo =
      {
         VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
         .renderPass = renderpass,
//         .framebuffer =,
      };

      const VkCommandBufferBeginInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT|VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
         .pInheritanceInfo = &inheritanceInfo,
      };
      vkBeginCommandBuffer(cmd, &info);
   }

   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipe);
//   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, sprite_renderer.layout, 0, 1, &sprite_renderer.desc, 0, NULL);
   vkCmdBindVertexBuffers(cmd, 0, 1, &renderer->vbo.info.buffer, &renderer->vbo.info.offset);
}

void vk_sprite_add(sprite_t *sprite, vk_texture_t *texture)
{
   *(sprite_t *)vk_get_vbo_memory(&sprite_renderer.vbo, sizeof(sprite_t)) = *sprite;

   if (texture->dirty && !texture->flushed)
      vk_texture_flush(device, texture);

   if (texture->dirty && !texture->uploaded)
      vk_texture_upload(device, current_main_cmd, texture);

   VkCommandBuffer cmd = sprite_renderer.cmd[current_cmd_id];

   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, sprite_renderer.layout, 0, 1, &texture->desc, 0, NULL);
   vkCmdDraw(cmd, 1, 1, (sprite_renderer.vbo.info.range - sizeof(sprite_t) - sprite_renderer.vbo.info.offset) / sizeof(sprite_t), 0);
}

void vk_sprite_emit(VkCommandBuffer cmd, vk_renderer_t *renderer)
{
   vkEndCommandBuffer(renderer->cmd[current_cmd_id]);

   if (renderer->vbo.info.range - renderer->vbo.info.offset == 0)
      return;

   vkCmdExecuteCommands(current_main_cmd, 1, &renderer->cmd[current_cmd_id++]);

   renderer->vbo.info.offset = renderer->vbo.info.range;
}

void vk_sprite_finish(VkDevice device, vk_renderer_t *renderer)
{
//   if (renderer->texture.dirty && !renderer->texture.flushed)
//      vk_texture_flush(device, &renderer->texture);

   if (renderer->vbo.dirty)
      vk_buffer_flush(device, &renderer->vbo);

   renderer->vbo.info.offset = 0;
   renderer->vbo.info.range = 0;
   current_cmd_id = 0;
   current_main_cmd = NULL;
}

vk_renderer_t sprite_renderer =
{
   .init = vk_sprite_renderer_init,
   .destroy = vk_renderer_destroy,
   .update = vk_sprite_update,
   .exec = vk_sprite_emit,
   .finish = vk_sprite_finish,
};

