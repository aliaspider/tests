
#include <string.h>
#include <assert.h>

#include "vulkan_common.h"
#include "frame.h"
#include "video.h"
#include "font.h"

typedef struct vertex_t
{
   vec2 position;
   vec2 size;
} vertex_t;

typedef struct uniform_t
{
   vec2 tex_size;
} uniform_t;


void vk_frame_init(vk_context_t *vk, int width, int height, VkFormat format)
{
   {
      const uint32_t vs_code [] =
#include "frame.vert.inc"
         ;
      const uint32_t ps_code [] =
#include "frame.frag.inc"
         ;

      const uint32_t gs_code [] =
#include "frame.geom.inc"
         ;

      const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {0, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, position)},
         {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, size)},
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

      frame_renderer.texture.width = width;
      frame_renderer.texture.height = height;
      frame_renderer.texture.format = format;
      frame_renderer.texture.filter = VK_FILTER_LINEAR;
      frame_renderer.vbo.info.range = sizeof(vertex_t) * 8;
      frame_renderer.vertex_stride = sizeof(vertex_t);
      frame_renderer.ubo.info.range = sizeof(uniform_t);

      vk_renderer_init(vk, &info, &frame_renderer);
   }

   {
      device_memory_t *mem = &frame_renderer.texture.staging.mem;
      memset(mem->u8 + mem->layout.offset, 0xFF, mem->layout.size - mem->layout.offset);
   }

   frame_renderer.texture.dirty = true;

   ((uniform_t*)frame_renderer.ubo.mem.ptr)->tex_size.width = frame_renderer.texture.width;
   ((uniform_t*)frame_renderer.ubo.mem.ptr)->tex_size.height = frame_renderer.texture.height;
   frame_renderer.ubo.dirty = true;

   video.frame.width = width;
   video.frame.height = height;
   video.frame.pitch = frame_renderer.texture.staging.mem.layout.rowPitch / 4;
   video.frame.data = frame_renderer.texture.staging.mem.u8 + frame_renderer.texture.staging.mem.layout.offset;
}

void vk_frame_add(int x, int y, int width, int height)
{
   vertex_t *v = vk_get_vbo_memory(&frame_renderer.vbo, sizeof(vertex_t));
   v->position.x = x;
   v->position.y = y;
   v->size.width = width;
   v->size.height = height;
}

vk_renderer_t frame_renderer =
{
   .destroy=vk_renderer_destroy,
   .update=vk_renderer_update,
   .exec=vk_renderer_emit,
   .finish=vk_renderer_finish,
};
