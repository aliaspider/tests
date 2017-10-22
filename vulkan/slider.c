
#include <string.h>

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

static vk_pipeline_t slider;

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
      const vk_pipeline_init_info_t info =
      {
         .shaders.vs.code = vs_code,
         .shaders.vs.code_size = sizeof(vs_code),
         .shaders.ps.code = ps_code,
         .shaders.ps.code_size = sizeof(ps_code),
         .shaders.gs.code = gs_code,
         .shaders.gs.code_size = sizeof(gs_code),
         .vertex_stride = sizeof(vertex_t),
         .attrib_count = countof(attrib_desc),
         .attrib_desc = attrib_desc,
         .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
         .color_blend_attachement_state = &color_blend_attachement_state,
      };

      slider.texture.width = 16;
      slider.texture.height = 16;
      slider.texture.format = VK_FORMAT_R8_UNORM;
      slider.vbo.info.range = 256 * sizeof(vertex_t);
      slider.ssbo.info.range = sizeof(ssbo_t);

      vk_pipeline_init(vk, &info, &slider);
      slider.vbo.info.range = 0;
      ((ssbo_t*)slider.ssbo.mem.ptr)->v[0].x = 0.66;
      buffer_flush(vk->device, &slider.ssbo);
   }
}

void vulkan_slider_add(int x, int y, int w, int h, float pos, float size)
{
//   slider.vbo.info.range = 0;
//   vertex_t* out = (vertex_t*)(slider.vbo.mem.u8 + slider.vbo.info.range);
//   out->x = x;
//   out->y = y;
//   out->w = w;
//   out->h = h;
//   out->pos = pos;
//   out->size = size;
////   out->x = 0;
////   out->y = 0;
////   out->w = 100;
////   out->h = 100;
////   out->pos = pos;
//   slider.vbo.info.range += sizeof(vertex_t);
//   slider.vbo.dirty = true;
}

void vulkan_slider_update(VkDevice device, VkCommandBuffer cmd)
{
   static float pos = 1.0;
   static float real_pos = 1.0;

   static bool grab = false;
   static pointer_t old_pointer;
//   static int count = 0;
//   printf("more %i\n", count++);

//   int console_len = console_get_len();
   const char** lines = vulkan_font_get_lines(console_get(), 0, 100, video.screens[0].width);
   int line_count = 0;
   while(lines[line_count])
      line_count++;

   int visible_lines = 33;
   float size = 1.0;
   if(visible_lines < line_count)
      size = (float)visible_lines / line_count;
   if(input.pointer.touch1 && !old_pointer.touch1)
   {
      if ((input.pointer.x > video.screens[1].width - 20)
          && (input.pointer.x < video.screens[1].width)
          && (input.pointer.y > video.screens[1].height * real_pos)
          && (input.pointer.y < video.screens[1].height * (real_pos + size)))
      {
         grab = true;
      }
   }
   else if(!input.pointer.touch1)
   {
      grab = false;
      pos = real_pos;
   }

   if(grab)
   {
      pos += (input.pointer.y - old_pointer.y) / (float)video.screens[1].height;
   }
   real_pos = pos > 0.0 ? pos < (1.0 - size) ? pos : 1.0 - size : 0.0;
   old_pointer = input.pointer;

   char buffer[512];
   snprintf(buffer, sizeof(buffer), "[%c]", grab ? '#' : ' ');
   vulkan_font_draw_text(buffer, 0, 40, video.screens[0].width);

   vulkan_font_draw_text(lines[(int)(line_count * real_pos)], 0, 100, video.screens[0].width);

   free(lines);
   vertex_t* out = (vertex_t*)(slider.vbo.mem.u8);
   out->x = video.screens[1].width - 20;
   out->y = 0;
   out->w = 20;
   out->h = video.screens[1].height;
   out->pos = real_pos;
   out->size = size;
   slider.vbo.info.range = sizeof(vertex_t);

//   if (slider.vbo.dirty)
      buffer_flush(device, &slider.vbo);


//   buffer_invalidate(device, &slider.ssbo);

//   DEBUG_VEC4(((ssbo_t*)slider.ssbo.mem.ptr)->v[0]);
//   DEBUG_VEC4(((ssbo_t*)slider.ssbo.mem.ptr)->v[1]);
//   DEBUG_VEC4(((ssbo_t*)slider.ssbo.mem.ptr)->v[2]);
//   DEBUG_VEC4(((ssbo_t*)slider.ssbo.mem.ptr)->v[3]);
}
void vulkan_slider_render(VkCommandBuffer cmd)
{

   VkDeviceSize offset = 0;
   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, slider.handle);
   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, slider.layout, 0, 1, &slider.desc, 0, NULL);
   vkCmdBindVertexBuffers(cmd, 0, 1, &slider.vbo.info.buffer, &offset);

   vkCmdDraw(cmd, slider.vbo.info.range / sizeof(vertex_t), 1, 0, 0);
}

void vulkan_slider_destroy(VkDevice device)
{
   vk_pipeline_destroy(device, &slider);
}
