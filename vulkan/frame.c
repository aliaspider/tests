
#include <string.h>

#include "vulkan_common.h"
#include "video.h"
#include "font.h"

static struct
{
   vk_texture_t tex;
   vk_buffer_t vbo;
   vk_buffer_t ubo;
   VkDescriptorSet desc;
   VkPipeline pipe;
   VkPipelineLayout pipe_layout;
   VkCommandBuffer cmd;
} frame;

typedef struct
{
   float val0;
   float val1;
} frame_uniforms_t;

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

      frame.vbo.info.range = sizeof(vertices);
      frame.vbo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      frame.vbo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
      buffer_init(vk->device, vk->memoryTypes, vertices, &frame.vbo);
   }

   frame.ubo.info.range = sizeof(frame_uniforms_t);
   frame.ubo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   frame.ubo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
   buffer_init(vk->device, vk->memoryTypes, NULL, &frame.ubo);

   {
      const VkDescriptorSetAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .descriptorPool = vk->pools.desc,
         .descriptorSetCount = 1, &vk_render->descriptor_set_layout
      };
      vkAllocateDescriptorSets(vk->device, &info, &frame.desc);
   }


   {
      VkShaderModule vertex_shader;
      {
         static const uint32_t code [] =
#include "frame.vert.inc"
            ;
         const VkShaderModuleCreateInfo info =
         {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof(code),
            .pCode = code
         };
         vkCreateShaderModule(vk->device, &info, NULL, &vertex_shader);
      }

      VkShaderModule fragment_shader;
      {
         static const uint32_t code [] =
#include "frame.frag.inc"
            ;
         const VkShaderModuleCreateInfo info =
         {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof(code),
            .pCode = code
         };
         vkCreateShaderModule(vk->device, &info, NULL, &fragment_shader);
      }

      const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, position)},
         {1, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(vertex_t, texcoord)},
         {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, color)}
      };

      {
         {
#if 0
            VkPushConstantRange ranges[] =
            {
               {
                  .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
                  .offset = 0,
                  .size = sizeof(frame_uniforms_t)
               }
            };
#endif
            const VkPipelineLayoutCreateInfo info =
            {
               VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
               .setLayoutCount = 1, &vk_render->descriptor_set_layout,
#if 0
               .pushConstantRangeCount = countof(ranges), ranges
#endif
            };

            vkCreatePipelineLayout(vk->device, &info, NULL, &frame.pipe_layout);
         }

         {
            const VkPipelineShaderStageCreateInfo shaders_info[] =
            {
               {
                  VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .stage = VK_SHADER_STAGE_VERTEX_BIT,
                  .pName = "main",
                  .module = vertex_shader
               },
               {
                  VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
                  .pName = "main",
                  .module = fragment_shader
               }
            };

            const VkVertexInputBindingDescription vertex_description =
            {
               0, sizeof(vertex_t), VK_VERTEX_INPUT_RATE_VERTEX
            };

            const VkPipelineVertexInputStateCreateInfo vertex_input_state =
            {
               VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
               .vertexBindingDescriptionCount = 1, &vertex_description,
               .vertexAttributeDescriptionCount = countof(attrib_desc), attrib_desc
            };

            const VkPipelineInputAssemblyStateCreateInfo input_assembly_state =
            {
               VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
               .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_FAN,
               .primitiveRestartEnable = VK_FALSE
            };

            const VkPipelineViewportStateCreateInfo viewport_state =
            {
               VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
               .viewportCount = 1, &vk_render->viewport,
               .scissorCount = 1, &vk_render->scissor
            };

            const VkPipelineRasterizationStateCreateInfo rasterization_info =
            {
               VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
               .lineWidth = 1.0f
            };

            const VkPipelineMultisampleStateCreateInfo multisample_state =
            {
               VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
               .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
            };

            const VkPipelineColorBlendAttachmentState attachement_state =
            {
               .blendEnable = VK_FALSE,
               .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
               VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
            };

            const VkPipelineColorBlendStateCreateInfo colorblend_state =
            {
               VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
               .attachmentCount = 1, &attachement_state
            };

            const VkGraphicsPipelineCreateInfo info =
            {
               VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
               .flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT,
               .stageCount = countof(shaders_info), shaders_info,
               .pVertexInputState = &vertex_input_state,
               .pInputAssemblyState = &input_assembly_state,
               .pViewportState = &viewport_state,
               .pRasterizationState = &rasterization_info,
               .pMultisampleState = &multisample_state,
               .pColorBlendState = &colorblend_state,
               .layout = frame.pipe_layout,
               .renderPass = vk_render->renderpass,
               .subpass = 0
            };
            vkCreateGraphicsPipelines(vk->device, VK_NULL_HANDLE, 1, &info, NULL, &frame.pipe);
         }

      }

      vkDestroyShaderModule(vk->device, vertex_shader, NULL);
      vkDestroyShaderModule(vk->device, fragment_shader, NULL);
   }

   {
      frame.tex.width = width;
      frame.tex.height = height;
      frame.tex.format = format;
      texture_init(vk->device, vk->memoryTypes, vk->queue_family_index, &frame.tex);

      /* texture updates are written to the stating texture then uploaded later */
      memset(frame.tex.staging.mem.u8 + frame.tex.staging.mem_layout.offset, 0xFF,
         frame.tex.staging.mem_layout.size - frame.tex.staging.mem_layout.offset);

      device_memory_flush(vk->device, &frame.tex.staging.mem);
      frame.tex.dirty = true;

      vk_update_descriptor_set(vk->device, &frame.tex, NULL, NULL, frame.desc);
   }

   video.frame.width = width;
   video.frame.height = height;
   video.frame.pitch = frame.tex.staging.mem_layout.rowPitch / 4;
   video.frame.data = frame.tex.staging.mem.u8 + frame.tex.staging.mem_layout.offset;

}

void vulkan_frame_update(VkDevice device, VkCommandBuffer cmd)
{
   device_memory_flush(device, &frame.tex.staging.mem);
   frame.tex.dirty = true;

   if (frame.tex.dirty)
      texture_update(cmd, &frame.tex);
}
void vulkan_frame_render(VkCommandBuffer cmd)
{
   VkDeviceSize offset = 0;
   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, frame.pipe);
   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, frame.pipe_layout, 0, 1, &frame.desc, 0, NULL);
// vkCmdPushConstants(device.cmd, device.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uniforms_t), mapped_uniforms);

   vkCmdBindVertexBuffers(cmd, 0, 1, &frame.vbo.info.buffer, &offset);

   vkCmdDraw(cmd, frame.vbo.info.range / sizeof(vertex_t), 1, 0, 0);
}
void vulkan_frame_destroy(VkDevice device)
{
   vkDestroyPipelineLayout(device, frame.pipe_layout, NULL);
   vkDestroyPipeline(device, frame.pipe, NULL);
   frame.pipe_layout = VK_NULL_HANDLE;
   frame.pipe = VK_NULL_HANDLE;

   buffer_free(device, &frame.vbo);
   texture_free(device, &frame.tex);
}
