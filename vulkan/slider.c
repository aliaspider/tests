
#include <string.h>
#include <math.h>

#include "vulkan_common.h"
#include "common.h"
#include "slider.h"
#include "input.h"
#include "font.h"

typedef struct
{
   float x;
   float y;
   float w;
   float h;
   float pos;
   float size;
} vertex_t;

typedef struct
{
   vec4 v[4];
}ssbo_t;

static vk_renderer_t slider_renderer;

void vulkan_slider_init(vk_context_t *vk)
{
   {
      const uint32_t vs_code [] =
#include "slider.vert.inc"
         ;
      const uint32_t ps_code [] =
#include "slider.frag.inc"
         ;

      const uint32_t gs_code [] =
#include "slider.geom.inc"
         ;

      const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, x)},
         {1, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, pos)},
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

      slider_renderer.texture.width = 16;
      slider_renderer.texture.height = 16;
      slider_renderer.texture.format = VK_FORMAT_R8_UNORM;
      slider_renderer.vbo.info.range = 256 * sizeof(vertex_t);
      slider_renderer.ssbo.info.range = sizeof(ssbo_t);
      slider_renderer.vertex_stride = sizeof(vertex_t);

      vk_renderer_init(vk, &info, &slider_renderer);
      slider_renderer.vbo.info.range = 0;
      ((ssbo_t*)slider_renderer.ssbo.mem.ptr)->v[0].x = 0.66;
      vk_buffer_flush(vk->device, &slider_renderer.ssbo);
   }
}


void vulkan_slider_add(int x, int y, int w, int h, float pos, float size)
{
   vertex_t* out = (vertex_t*)(slider_renderer.vbo.mem.u8 + slider_renderer.vbo.info.range);
   out->x = x;
   out->y = y;
   out->w = w;
   out->h = h;
   out->pos = pos;
   out->size = size;

   slider_renderer.vbo.info.range += sizeof(vertex_t);
   slider_renderer.vbo.dirty = true;
   assert(slider_renderer.vbo.info.range <= slider_renderer.vbo.mem.size);
}

void vulkan_slider_start(void)
{
   slider_renderer.vbo.info.offset = 0;
   slider_renderer.vbo.info.range = 0;
//   slider_renderer.texture.flushed = false;
//   slider_renderer.texture.uploaded = false;
}

void vulkan_slider_finish(VkDevice device)
{
   if(slider_renderer.vbo.dirty)
      vk_buffer_flush(device, &slider_renderer.vbo);

}

void vulkan_slider_update(VkDevice device, VkCommandBuffer cmd)
{
//   if (slider_renderer.texture.dirty && !slider_renderer.texture.uploaded)
//      vk_texture_upload(device, cmd, &slider_renderer.texture);
}


void vulkan_slider_render(VkCommandBuffer cmd)
{
   vk_renderer_draw(cmd, &slider_renderer);
}

void vulkan_slider_destroy(VkDevice device)
{
   vk_renderer_destroy(device, &slider_renderer);
}
