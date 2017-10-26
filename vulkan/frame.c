
#include <string.h>
#include <assert.h>

#include "vulkan_common.h"
#include "frame.h"
#include "video.h"
#include "font.h"

typedef struct
{
   struct
   {
      float x, y, z, w;
   } position;
   struct
   {
      float u, v;
   } texcoord;
   struct
   {
      float r, g, b, a;
   } color;
} vertex_t;

static vk_renderer_t frame_renderer;

void vulkan_frame_init(vk_context_t *vk, int width, int height, VkFormat format)
{
   {
      const uint32_t vs_code [] =
#include "frame.vert.inc"
         ;
      const uint32_t ps_code [] =
#include "frame.frag.inc"
         ;

      const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, position)},
         {1, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(vertex_t, texcoord)},
         {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, color)}
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
         .attrib_count = countof(attrib_desc),
         .attrib_desc = attrib_desc,
         .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
         .color_blend_attachement_state = &color_blend_attachement_state,
      };

      frame_renderer.texture.width = width;
      frame_renderer.texture.height = height;
      frame_renderer.texture.format = format;
      frame_renderer.vbo.info.range = sizeof(vertex_t) * 4 * 8;
      frame_renderer.vertex_stride = sizeof(vertex_t);

      vk_renderer_init(vk, &info, &frame_renderer);
      frame_renderer.vbo.info.range = 0;
      frame_renderer.texture.dirty = true;
   }

   memset(frame_renderer.texture.staging.mem.u8 + frame_renderer.texture.staging.mem.layout.offset, 0xFF,
      frame_renderer.texture.staging.mem.layout.size - frame_renderer.texture.staging.mem.layout.offset);
   frame_renderer.texture.dirty = true;

   video.frame.width = width;
   video.frame.height = height;
   video.frame.pitch = frame_renderer.texture.staging.mem.layout.rowPitch / 4;
   video.frame.data = frame_renderer.texture.staging.mem.u8 + frame_renderer.texture.staging.mem.layout.offset;
}

void vulkan_frame_add(int x, int y, int width, int height)
{
   const vertex_t vertices[] =
   {
      {{ -1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
      {{ 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
      {{ 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
      {{ -1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}
   };
   memcpy(frame_renderer.vbo.mem.u8 + frame_renderer.vbo.info.range, vertices, sizeof(vertices));
   frame_renderer.vbo.info.range += sizeof(vertices);
   frame_renderer.vbo.dirty = true;
   assert(frame_renderer.vbo.info.range <= frame_renderer.vbo.mem.size);
}

void vulkan_frame_finish(VkDevice device)
{
   if (frame_renderer.texture.dirty && !frame_renderer.texture.flushed)
      vk_texture_flush(device, &frame_renderer.texture);

   if(frame_renderer.vbo.dirty)
      vk_buffer_flush(device, &frame_renderer.vbo);

   frame_renderer.vbo.info.offset = 0;
   frame_renderer.vbo.info.range = 0;
   frame_renderer.texture.dirty = true;
   frame_renderer.texture.flushed = false;
   frame_renderer.texture.uploaded = false;
}

void vulkan_frame_update(VkDevice device, VkCommandBuffer cmd)
{
   if (frame_renderer.texture.dirty && !frame_renderer.texture.uploaded)
      vk_texture_upload(device, cmd, &frame_renderer.texture);

}

void vulkan_frame_render(VkCommandBuffer cmd)
{
   vk_renderer_draw(cmd, &frame_renderer);
}

void vulkan_frame_destroy(VkDevice device)
{
   vk_renderer_destroy(device, &frame_renderer);
}
