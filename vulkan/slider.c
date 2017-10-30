
#include <string.h>
#include <math.h>

#include "vulkan_common.h"
//#include "common.h"
#include "slider.h"

typedef struct
{
   float x;
   float y;
   float w;
   float h;
   float pos;
   float size;
} vertex_t;

static void vk_slider_init(vk_context_t *vk)
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

   R_slider.vbo.info.range = 256 * sizeof(vertex_t);
   R_slider.vertex_stride = sizeof(vertex_t);

   vk_renderer_init(vk, &info, &R_slider);
}


void vk_slider_add(int x, int y, int w, int h, float pos, float size)
{
   vertex_t *out = (vertex_t *)vk_get_vbo_memory(&R_slider.vbo, sizeof(vertex_t));
   out->x = x;
   out->y = y;
   out->w = w;
   out->h = h;
   out->pos = pos;
   out->size = size;
}

vk_renderer_t R_slider =
{
   .init=vk_slider_init,
   .destroy=vk_renderer_destroy,
   .update=vk_renderer_update,
   .exec=vk_renderer_exec,
   .finish=vk_renderer_finish,
};

