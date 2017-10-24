
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
   static bool grab = false;
   static pointer_t old_pointer;
//   static int count = 0;
//   printf("more %i\n", count++);
   if(input.pointer.x < video.screens[1].width - 20&& !input.pointer.touch1 && old_pointer.touch1)
      printf("click\n");

   string_list_t* lines = string_list_create();
   font_render_options_t options =
   {
      .y = 100,
      .max_width = video.screens[1].width - 20,
      .max_height = video.screens[1].height,
      .lines = lines,
      .dry_run = true
   };
   vulkan_font_draw_text(console_get(), &options);
   options.lines = NULL;
   options.dry_run = false;

   int visible_lines = 33;
   float size;
   if(visible_lines < lines->count)
      size = (float)visible_lines / lines->count;
   else
      size = 1.0;

   if(input.pointer.touch1 && !old_pointer.touch1)
   {
      if ((input.pointer.x > video.screens[1].width - 20)
          && (input.pointer.x < video.screens[1].width))
      {
         if(!grab)
         {
            if (input.pointer.y < video.screens[1].height * (pos * (1.0 - size)))
            {
               pos = input.pointer.y / ((float)video.screens[1].height * (1.0 - size)) - (size / 2) / (1.0 - size);
               pos = pos > 0.0 ? pos < 1.0 ? pos : 1.0 : 0.0;
            }
            else if (input.pointer.y > video.screens[1].height * (pos * (1.0 - size) + size))
            {
               pos = input.pointer.y / ((float)video.screens[1].height * (1.0 - size)) - (size / 2) / (1.0 - size);
               pos = pos > 0.0 ? pos < 1.0 ? pos : 1.0 : 0.0;
            }
         }
         grab = true;
      }      
   }
   else if(!input.pointer.touch1)
   {
      grab = false;
      pos = pos > 0.0 ? pos < 1.0 ? pos : 1.0 : 0.0;
   }

   if(grab)
   {
      pos += (input.pointer.y - old_pointer.y) / ((float)video.screens[1].height * (1.0 - size));
   }
   float real_pos = pos > 0.0 ? pos < 1.0 ? pos * (1.0 - size) : 1.0 - size : 0.0;

   old_pointer = input.pointer;

   char buffer[512];
   snprintf(buffer, sizeof(buffer), "[%c]", grab ? '#' : ' ');
   options.y = 40;
   vulkan_font_draw_text(buffer, &options);

   options.y = 100;
   options.max_width = video.screens[0].width - 20;
   vulkan_font_draw_text(lines->data[(int)(0.5 + lines->count * real_pos)], &options);

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
