
#include <string.h>
#include <assert.h>

#include "vulkan_common.h"
#include "common.h"
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


static void vk_frame_init(vk_context_t *vk)
{
   VkFormat format;

   switch (module.screen_format)
   {
   case screen_format_RGB565:
      format = VK_FORMAT_R5G6B5_UNORM_PACK16;
      break;

   case screen_format_ARGB5551:
      format = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
      break;

   default:
      format = VK_FORMAT_R8G8B8A8_UNORM;
      break;
   }

   {

#define SHADER_FILE frame
#include "shaders.h"

      static const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {.location = 0, .binding = 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, position)},
         {.location = 1, .binding = 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, size)},
      };

      static const VkPipelineColorBlendAttachmentState blend_state =
      {
         .blendEnable = VK_FALSE,
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

      R_frame.tex.width = module.output_width;
      R_frame.tex.height = module.output_height;
      R_frame.tex.format = format;
      R_frame.tex.filter = video.filter ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
      R_frame.tex.ignore_alpha = true;
      R_frame.vbo.info.range = sizeof(vertex_t) * 8;
      R_frame.vertex_stride = sizeof(vertex_t);
      R_frame.ubo.info.range = sizeof(uniform_t);

      vk_renderer_init(vk, &info, &R_frame);
   }

   {
      device_memory_t *mem = &R_frame.tex.staging.mem;
      memset(mem->u8 + mem->layout.offset, 0xFF, mem->layout.size - mem->layout.offset);
   }

   R_frame.tex.dirty = true;

   ((uniform_t *)R_frame.ubo.mem.ptr)->tex_size.width = R_frame.tex.width;
   ((uniform_t *)R_frame.ubo.mem.ptr)->tex_size.height = R_frame.tex.height;
   R_frame.ubo.dirty = true;

   video.frame.width = R_frame.tex.width;
   video.frame.height = R_frame.tex.height;
   video.frame.pitch = R_frame.tex.staging.mem.layout.rowPitch / 4;
   video.frame.data = R_frame.tex.staging.mem.u8 + R_frame.tex.staging.mem.layout.offset;
}

void vk_frame_add(int x, int y, int width, int height)
{
   vertex_t *v = vk_get_vbo_memory(&R_frame.vbo, sizeof(vertex_t));
   v->position.x = x;
   v->position.y = y;
   v->size.width = width;
   v->size.height = height;
}

vk_renderer_t R_frame =
{
   .init = vk_frame_init,
   .destroy = vk_renderer_destroy,
   .begin = vk_renderer_begin,
   .finish = vk_renderer_finish,
};
