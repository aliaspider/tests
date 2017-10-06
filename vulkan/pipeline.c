
#include "vulkan_common.h"

void pipeline_init(VkDevice device, const pipeline_init_info_t *init_info, pipeline_t* dst)
{
   {
#if 0
      VkPushConstantRange ranges[] =
      {
         {
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
            .offset = 0,
            .size = sizeof(uniforms_t)
         }
      };
#endif
      const VkPipelineLayoutCreateInfo info =
      {
         VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
         .setLayoutCount = 1, &init_info->set_layout,
#if 0
         .pushConstantRangeCount = countof(ranges), ranges
#endif
      };

      vkCreatePipelineLayout(device, &info, NULL, &dst->layout);
   }

   {
      const VkPipelineShaderStageCreateInfo shaders_info[] =
      {
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .pName = "main",
            .module = init_info->vertex_shader
         },
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pName = "main",
            .module = init_info->fragment_shader
         }
      };

      const VkVertexInputBindingDescription vertex_description =
      {
         0, init_info->vertex_size, VK_VERTEX_INPUT_RATE_VERTEX
      };

      const VkPipelineVertexInputStateCreateInfo vertex_input_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
         .vertexBindingDescriptionCount = 1, &vertex_description,
         .vertexAttributeDescriptionCount = init_info->attrib_count, init_info->attrib_desc
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
         .viewportCount = 1, init_info->viewport,
         .scissorCount = 1, init_info->scissor
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
         .layout = dst->layout,
         .renderPass = init_info->renderpass,
         .subpass = 0
      };
      vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, NULL, &dst->handle);
   }
}

void pipeline_free(VkDevice device, pipeline_t *pipe)
{
   vkDestroyPipelineLayout(device, pipe->layout, NULL);
   vkDestroyPipeline(device, pipe->handle, NULL);
   pipe->layout = VK_NULL_HANDLE;
   pipe->handle = VK_NULL_HANDLE;
}
