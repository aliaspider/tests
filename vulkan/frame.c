
#include <string.h>

#include "vulkan_common.h"
#include "video.h"
#include "font.h"

typedef struct
{
   float val0;
   float val1;
} frame_uniforms_t;

static vk_render_t frame;

void vulkan_frame_init(vk_context_t *vk, vk_render_context_t *vk_render, int width, int height, VkFormat format)
{
   {
      const vertex_t vertices[] =
      {
         {{ -1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
         {{ 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
         {{ 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
         {{ -1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}
      };
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
      const vk_pipeline_init_info_t info =
      {
         .shaders.vs.code = vs_code,
         .shaders.vs.code_size = sizeof(vs_code),
         .shaders.ps.code = ps_code,
         .shaders.ps.code_size = sizeof(ps_code),
         .vertex_stride = sizeof(vertex_t),
         .attrib_count = countof(attrib_desc),
         .attrib_desc = attrib_desc,
         .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
         .color_blend_attachement_state = &color_blend_attachement_state,
      };

      frame.texture.width = width;
      frame.texture.height = height;
      frame.texture.format = format;
      frame.ubo.info.range = sizeof(frame_uniforms_t);
      frame.vbo.info.range = sizeof(vertices);

      vk_render_init(vk, vk_render, &info, &frame);

      memcpy(frame.vbo.mem.ptr, vertices, sizeof(vertices));
      frame.vbo.dirty = true;
   }

   memset(frame.texture.staging.mem.u8 + frame.texture.staging.mem_layout.offset, 0xFF,
      frame.texture.staging.mem_layout.size - frame.texture.staging.mem_layout.offset);
   frame.texture.dirty = true;

   video.frame.width = width;
   video.frame.height = height;
   video.frame.pitch = frame.texture.staging.mem_layout.rowPitch / 4;
   video.frame.data = frame.texture.staging.mem.u8 + frame.texture.staging.mem_layout.offset;
}

void vulkan_frame_update(VkDevice device, VkCommandBuffer cmd)
{
   texture_update(device, cmd, &frame.texture);
}

void vulkan_frame_render(VkCommandBuffer cmd)
{
   VkDeviceSize offset = 0;
   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, frame.pipe);
   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, frame.pipeline_layout, 0, 1, &frame.desc, 0, NULL);
// vkCmdPushConstants(device.cmd, device.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uniforms_t), mapped_uniforms);

   vkCmdBindVertexBuffers(cmd, 0, 1, &frame.vbo.info.buffer, &offset);

   vkCmdDraw(cmd, frame.vbo.info.range / sizeof(vertex_t), 1, 0, 0);
}

void vulkan_frame_destroy(VkDevice device)
{
   vkDestroyPipelineLayout(device, frame.pipeline_layout, NULL);
   vkDestroyPipeline(device, frame.pipe, NULL);

   buffer_free(device, &frame.vbo);
   buffer_free(device, &frame.ubo);
   texture_free(device, &frame.texture);

   memset(&frame, 0, sizeof(frame));
}
