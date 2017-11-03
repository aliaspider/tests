
#include <string.h>
#include <math.h>

#include "vulkan_common.h"
#include "common.h"
#include "slider.h"
#include "utils/png_file.h"

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
#define SHADER_FILE slider
#include "shaders.h"

   static const VkVertexInputAttributeDescription attrib_desc[] =
   {
      {.location = 0, .binding = 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, x)},
      {.location = 1, .binding = 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, pos)},
   };

   static const VkPipelineColorBlendAttachmentState blend_state =
   {
      .blendEnable = VK_TRUE,
      .srcColorBlendFactor = VK_SRC_ALPHA, VK_ONE_MINUS_SRC_ALPHA, VK_ADD,
      .srcAlphaBlendFactor = VK_SRC_ALPHA, VK_ONE_MINUS_SRC_ALPHA, VK_ADD,
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

   png_file_t png;
   png_file_init("vulkan/slider.png", &png);

   R_slider.default_texture.width = png.width;
   R_slider.default_texture.height = png.height;
   R_slider.default_texture.format = VK_FORMAT_R8G8B8A8_UNORM;
   R_slider.vbo.info.range = 256 * sizeof(vertex_t);
   R_slider.vertex_stride = sizeof(vertex_t);

   vk_renderer_init(vk, &info, &R_slider);

   device_memory_t *mem = &R_slider.default_texture.staging.mem;
   memset(mem->u8 + mem->layout.offset, 0xFF, mem->layout.size - mem->layout.offset);

   png_file_read(&png, R_slider.default_texture.staging.mem.ptr+ mem->layout.offset, R_slider.default_texture.staging.mem.layout.rowPitch);
   R_slider.default_texture.dirty = true;
   png_file_free(&png);
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
   .begin = vk_renderer_begin,
   .finish =vk_renderer_finish,
};

