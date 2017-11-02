
#include <string.h>

#include "renderer.h"
#include "inlines.h"

static inline VkShaderModule vk_shader_code_init(VkDevice device, const vk_shader_code_t *shader_code)
{
   VkShaderModule shader;

   const VkShaderModuleCreateInfo info =
   {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = shader_code->code_size,
      .pCode = shader_code->code
   };
   vkCreateShaderModule(device, &info, NULL, &shader);

   return shader;
}

static void vk_update_descriptor_sets(vk_context_t *vk, vk_renderer_t *out)
{
   VkWriteDescriptorSet write_set[3];
   int write_set_count = 0;

   VkDescriptorImageInfo image_info[] =
   {
      {
         .sampler = out->default_texture.info.sampler,
         .imageView = out->default_texture.info.imageView,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      },
      {
         .sampler = out->default_texture.info.sampler,
         .imageView = out->default_texture.info.imageView,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      }
   };

   if (out->default_texture.image)
   {
      VkWriteDescriptorSet set =
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = out->desc.main,
         .dstBinding = 0,
         .dstArrayElement = 0,
         .descriptorCount = countof(image_info),
         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .pImageInfo = image_info
      };
      write_set[write_set_count++] = set;
   }

   if (out->ubo.info.buffer)
   {
      VkWriteDescriptorSet set =
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = out->desc.main,
         .dstBinding = 1,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .pBufferInfo = &out->ubo.info
      };
      write_set[write_set_count++] = set;
   }

   if (out->ssbo.info.buffer)
   {
      VkWriteDescriptorSet set =
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = out->desc.main,
         .dstBinding = 2,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .pBufferInfo = &out->ssbo.info
      };
      write_set[write_set_count++] = set;
   }

   vkUpdateDescriptorSets(vk->device, write_set_count, write_set, 0, NULL);
}

void vk_renderer_init(vk_context_t *vk, const vk_renderer_init_info_t *init_info, vk_renderer_t *out)
{
   if (out->default_texture.image)
      out->default_texture.is_reference = true;
   else
      vk_texture_init(vk, &out->default_texture);

   if (out->ssbo.info.range)
   {
      out->ssbo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      out->ssbo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
      vk_buffer_init(vk->device, vk->memoryTypes, NULL, &out->ssbo);
   }

   if (out->ubo.info.range)
   {
      out->ubo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      out->ubo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      vk_buffer_init(vk->device, vk->memoryTypes, NULL, &out->ubo);
   }

   out->vbo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   out->vbo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
   vk_buffer_init(vk->device, vk->memoryTypes, NULL, &out->vbo);
   out->vbo.info.range = 0;

   {
      const VkDescriptorSetAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .descriptorPool = vk->pools.desc,
         .descriptorSetCount = 1, &vk->set_layouts.renderer
      };
      vkAllocateDescriptorSets(vk->device, &info, &out->desc.main);
   }
   vk_update_descriptor_sets(vk, out);

   out->desc.texture = out->default_texture.desc;

   VkAllocateCommandBuffers(vk->device, vk->pools.cmd, VK_COMMAND_BUFFER_LEVEL_SECONDARY, MAX_SCREENS, out->cmds);

   out->pipeline_layout = vk->pipeline_layout;
   out->renderpass = vk->renderpass;

   {
      const VkPipelineShaderStageCreateInfo shaders_info[] =
      {
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .pName = "main",
            .module = vk_shader_code_init(vk->device, &init_info->shaders.vs)
         },
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pName = "main",
            .module = vk_shader_code_init(vk->device, &init_info->shaders.ps)
         },
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
            .pName = "main",
            .module = init_info->shaders.gs.code ? vk_shader_code_init(vk->device, &init_info->shaders.gs) : VK_NULL_HANDLE
         }
      };

      const VkVertexInputBindingDescription vertex_description =
      {
         0, out->vertex_stride, VK_VERTEX_INPUT_RATE_VERTEX
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
         .topology = init_info->topology,
         .primitiveRestartEnable = VK_FALSE
      };

      static const VkPipelineViewportStateCreateInfo viewport_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
         .viewportCount = 1, .scissorCount = 1
      };

      static const VkPipelineRasterizationStateCreateInfo rasterization_info =
      {
         VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
//         .rasterizerDiscardEnable = VK_TRUE,
         .lineWidth = 1.0f
      };

      static const VkPipelineMultisampleStateCreateInfo multisample_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
         .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      };

      const VkPipelineColorBlendStateCreateInfo colorblend_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
         .attachmentCount = 1, init_info->color_blend_attachement_state
      };

      static const VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
      static const VkPipelineDynamicStateCreateInfo dynamic_state_info =
      {
         VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
         .dynamicStateCount = countof(dynamic_states), dynamic_states
      };

      VkGraphicsPipelineCreateInfo info[MAX_SCREENS] =
      {
         {
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT | VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
            .stageCount = shaders_info[2].module ? 3 : 2, shaders_info,
            .pVertexInputState = &vertex_input_state,
            .pInputAssemblyState = &input_assembly_state,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterization_info,
            .pMultisampleState = &multisample_state,
            .pColorBlendState = &colorblend_state,
            .pDynamicState = &dynamic_state_info,
            .layout = out->pipeline_layout,
            .renderPass = out->renderpass,
            .subpass = 0
         }
      };
      vkCreateGraphicsPipelines(vk->device, VK_NULL_HANDLE, 1, info, NULL, &out->pipe);

      vkDestroyShaderModule(vk->device, shaders_info[0].module, NULL);
      vkDestroyShaderModule(vk->device, shaders_info[1].module, NULL);

      if (shaders_info[2].module)
         vkDestroyShaderModule(vk->device, shaders_info[2].module, NULL);
   }

}

void vk_renderer_destroy(vk_renderer_t *renderer, VkDevice device)
{
   vkDestroyPipeline(device, renderer->pipe, NULL);
   vk_buffer_free(device, &renderer->vbo);
   vk_buffer_free(device, &renderer->ubo);
   vk_buffer_free(device, &renderer->ssbo);
   vk_texture_free(device, &renderer->default_texture);

   memset(&renderer->vk_renderer_data_start, 0, sizeof(*renderer) - offsetof(vk_renderer_t, vk_renderer_data_start));
}

void vk_renderer_begin(vk_renderer_t *renderer, screen_t *screen)
{
   if (screen->id == 0)
      renderer->vbo.info.offset = 0;

   assert(!renderer->vbo.info.range);
   renderer->first_vertex = 0;
   renderer->desc.texture = renderer->default_texture.desc;
   renderer->cmd = renderer->cmds[screen->id];

   VkBeginCommandBuffer(renderer->cmd, renderer->renderpass, VK_ONE_TIME_SUBMIT | VK_RENDER_PASS_CONTINUE);
   vkCmdBindPipeline(renderer->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipe);
   vkCmdBindDescriptorSets(renderer->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline_layout, 1,
                           sizeof(renderer->desc) / sizeof(VkDescriptorSet), (VkDescriptorSet*)&renderer->desc, 0, NULL);
   vkCmdBindVertexBuffers(renderer->cmd, 0, 1, &renderer->vbo.info.buffer, &renderer->vbo.info.offset);
}

static inline void vk_renderer_flush_vertices(vk_renderer_t *renderer)
{
   if (renderer->vbo.info.range)
   {
      assert(renderer->vbo.info.range < 0x10000);

      int count = renderer->vbo.info.range / renderer->vertex_stride;

      vkCmdDraw(renderer->cmd, count, 1, renderer->first_vertex, 0);

      renderer->first_vertex += count;
      renderer->vbo.info.offset += renderer->vbo.info.range;
      renderer->vbo.info.range = 0;
   }
}

void vk_renderer_bind_texture(vk_renderer_t *renderer, vk_texture_t *texture)
{
   if(!texture)
      texture = &renderer->default_texture;

   if(renderer->desc.texture != texture->desc)
   {
      vk_renderer_flush_vertices(renderer);

      renderer->desc.texture = texture->desc;
      vkCmdBindDescriptorSets(renderer->cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipeline_layout, 2, 1, &renderer->desc.texture, 0, NULL);
   }
}

VkCommandBuffer vk_renderer_finish(vk_renderer_t *renderer)
{
   vk_renderer_flush_vertices(renderer);
   vkEndCommandBuffer(renderer->cmd);

   return renderer->cmd;
}
