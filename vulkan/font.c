
#include <ctype.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "vulkan_common.h"
#include "font.h"
#include "video.h"

#define VK_ATLAS_WIDTH  640
#define VK_ATLAS_HEIGHT 480
static vk_texture_t atlas;
static VkDescriptorSet atlas_desc;
static FT_Library ftlib;
static vk_buffer_t atlas_vbo;
static vk_buffer_t atlas_g_buffer;
static VkPipeline pipe;
static VkPipelineLayout pipe_layout;


typedef struct
{
   int last_id;
   int pos_x;
   int pos_y;
}font_shader_storage_t;

typedef struct
{
   uint8_t id;
   struct
   {
      uint8_t r;
      uint8_t g;
      uint8_t b;
   }color;
   struct
   {
      float x;
      float y;
   }position;
}font_vertex_t;

void vulkan_font_init(VkDevice device, uint32_t queue_family_index, const VkMemoryType *memory_types,
                      VkDescriptorPool descriptor_pool, VkDescriptorSetLayout descriptor_set_layout,
                      const VkRect2D *scissor, const VkViewport *viewport, VkRenderPass renderpass)
{
   {
      texture_init_info_t info =
      {
         .queue_family_index = queue_family_index,
         .width = VK_ATLAS_WIDTH,
         .height = VK_ATLAS_HEIGHT,
         .format = VK_FORMAT_R8_UNORM,
         .filter = VK_FILTER_NEAREST
      };
      texture_init(device, memory_types, &info, &atlas);
   }

   {
      buffer_init_info_t info =
      {
         .usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT,
         .req_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
         .size = sizeof(font_shader_storage_t),
      };
      buffer_init(device, memory_types, &info, &atlas_g_buffer);
   }

   {
      const VkDescriptorSetAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .descriptorPool = descriptor_pool,
         .descriptorSetCount = 1, &descriptor_set_layout
      };
      vkAllocateDescriptorSets(device, &info, &atlas_desc);
   }


   {
      const VkDescriptorBufferInfo buffer_info =
      {
         .buffer = atlas_g_buffer.handle,
         .offset = 0,
         .range = sizeof(font_shader_storage_t)
      };
      const VkDescriptorImageInfo image_info =
      {
         .sampler = atlas.sampler,
         .imageView = atlas.view,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      };

      const VkWriteDescriptorSet write_set[] =
      {
         {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = atlas_desc,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pBufferInfo = &buffer_info
         },
         {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = atlas_desc,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &image_info
         }
      };
      vkUpdateDescriptorSets(device, countof(write_set), write_set, 0, NULL);
   }


   memset(atlas.staging.mem.u8 + atlas.staging.mem_layout.offset, 0x0,
          atlas.staging.mem_layout.size - atlas.staging.mem_layout.offset);

   atlas.dirty = true;

   {
      FT_Face ftface;
      const char* font_path = "/usr/share/fonts/TTF/DejaVuSansMono.ttf";
//      const char* font_path = "/usr/share/fonts/TTF/LiberationMono-Regular.ttf";
//      const char* font_path = "/usr/share/fonts/75dpi/charR12.pcf.gz";
//      const char* font_path = "/usr/share/fonts/WindowsFonts/cour.ttf";


      FT_UInt font_size = 26;

      FT_Init_FreeType(&ftlib);
      FT_New_Face(ftlib, font_path, 0, &ftface);
      FT_Select_Charmap(ftface, FT_ENCODING_UNICODE);
      FT_Set_Pixel_Sizes(ftface, 0, font_size);

//#define FT_LOAD_MODE FT_LOAD_MONOCHROME
#define FT_LOAD_MODE 0
      int i;
      for (i = 0; i < 256; i++)
//         if(isalnum(i))
         {
            FT_Load_Char(ftface, i, FT_LOAD_RENDER|FT_LOAD_MODE);

//            FT_Render_Glyph(ftface->glyph, FT_RENDER_MODE_MONO);

            uint8_t* src = ftface->glyph->bitmap.buffer;
            uint8_t* dst = atlas.staging.mem.u8 + atlas.staging.mem_layout.offset +
                           (i&0xF) * (VK_ATLAS_WIDTH / 16) + ftface->glyph->bitmap_left +
                           (((i >> 4) * (VK_ATLAS_HEIGHT / 16)) + font_size - ftface->glyph->bitmap_top) * VK_ATLAS_WIDTH;
            int row;
            for(row = 0; row < ftface->glyph->bitmap.rows; row++)
            {
#if (FT_LOAD_MODE==FT_LOAD_MONOCHROME)
               int col;
               for(col = 0; col < ftface->glyph->bitmap.width; col++)
               {
                  if(src[col >> 3] & (0x80 >> (col&0x7)))
                     dst[col] = 255;
               }
#else
               memcpy(dst, src, ftface->glyph->bitmap.width);
#endif
               src += ftface->glyph->bitmap.pitch;
               dst += atlas.staging.mem_layout.rowPitch;
            }
         }

      FT_Done_Face(ftface);
      FT_Done_FreeType(ftlib);
   }

   device_memory_flush(device, &atlas.staging.mem);

   {
      buffer_init_info_t info =
      {
         .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         .req_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
         .size = 4096 * sizeof(font_vertex_t)
      };
      buffer_init(device, memory_types, &info, &atlas_vbo);      
      atlas_vbo.size = 0;
   }

   {
      VkShaderModule vertex_shader;
      {
         static const uint32_t code [] =
#include "font.vert.inc"
            ;
         const VkShaderModuleCreateInfo info =
         {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof(code),
            .pCode = code
         };
         vkCreateShaderModule(device, &info, NULL, &vertex_shader);
      }

      VkShaderModule fragment_shader;
      {
         static const uint32_t code [] =
#include "font.frag.inc"
            ;
         const VkShaderModuleCreateInfo info =
         {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof(code),
            .pCode = code
         };
         vkCreateShaderModule(device, &info, NULL, &fragment_shader);
      }

      VkShaderModule geometry_shader;
      {
         static const uint32_t code [] =
#include "font.geom.inc"
            ;
         const VkShaderModuleCreateInfo info =
         {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof(code),
            .pCode = code
         };
         vkCreateShaderModule(device, &info, NULL, &geometry_shader);
      }

      const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {0, 0, VK_FORMAT_R8_UINT, offsetof(font_vertex_t, id)},
         {1, 0, VK_FORMAT_R8G8B8_UNORM, offsetof(font_vertex_t, color)},
         {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(font_vertex_t, position)}
      };

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
               .setLayoutCount = 1, &descriptor_set_layout,
#if 0
               .pushConstantRangeCount = countof(ranges), ranges
#endif
            };

            vkCreatePipelineLayout(device, &info, NULL, &pipe_layout);
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
               },
               {
                  VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
                  .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
                  .pName = "main",
                  .module = geometry_shader
               }
            };

            const VkVertexInputBindingDescription vertex_description =
            {
               0, sizeof(font_vertex_t), VK_VERTEX_INPUT_RATE_VERTEX
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
               .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
               .primitiveRestartEnable = VK_FALSE
            };

            const VkPipelineViewportStateCreateInfo viewport_state =
            {
               VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
               .viewportCount = 1, viewport,
               .scissorCount = 1, scissor
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
               .blendEnable = VK_TRUE,
               .srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
               .dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
               .colorBlendOp = VK_BLEND_OP_ADD,
               .srcAlphaBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA,
               .dstAlphaBlendFactor =VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
               .alphaBlendOp = VK_BLEND_OP_ADD,
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
               .layout = pipe_layout,
               .renderPass = renderpass,
               .subpass = 0
            };
            vkCreateGraphicsPipelines(device, VK_NULL_HANDLE, 1, &info, NULL, &pipe);
         }

      }

      vkDestroyShaderModule(device, vertex_shader, NULL);
      vkDestroyShaderModule(device, fragment_shader, NULL);
      vkDestroyShaderModule(device, geometry_shader, NULL);
   }


}

void vulkan_font_destroy(VkDevice device, VkDescriptorPool pool)
{
   buffer_free(device, &atlas_vbo);
   texture_free(device, &atlas);
   vkDestroyPipelineLayout(device, pipe_layout, NULL);
   vkDestroyPipeline(device, pipe, NULL);
   pipe_layout = VK_NULL_HANDLE;
   pipe = VK_NULL_HANDLE;

//   vkFreeDescriptorSets(device, pool, 1, &atlas_desc);
   atlas_desc = VK_NULL_HANDLE;

}

void vulkan_font_draw_text(const char* text, int x, int y)
{
   const char* in = text;
   font_vertex_t* out = (font_vertex_t*)(atlas_vbo.mem.u8 + atlas_vbo.size);
   font_vertex_t vertex;
   vertex.color.r = 255;
   vertex.color.g = 255;
   vertex.color.b = 255;
   vertex.position.x = 8 + x;
   vertex.position.y = 16 + y;
   vertex.id = 'p';
   while(*in)
   {
      if(*in == '\n')
      {
         vertex.position.x = 8;
         vertex.position.y += 32;
         in++;
         continue;
      }
      *out = vertex;
      vertex.position.x += 16;
      if((vertex.position.x + 8) > video.screen.width)
      {
         vertex.position.x = 8;
         vertex.position.y += 32;
      }
      (out++)->id = *(in++);
   }

   atlas_vbo.size = (uint8_t*)out - atlas_vbo.mem.u8;


}

void vulkan_font_update_assets(VkCommandBuffer cmd)
{
   atlas_vbo.size = 0;

   vulkan_font_draw_text("test 1", 0, 0);
   vulkan_font_draw_text("test 2", 0, 32);
   vulkan_font_draw_text("test 3", 40, 220);
   if (atlas.dirty)
      texture_update(cmd, &atlas);

   if(atlas_vbo.dirty)
   {
      VkMappedMemoryRange range =
      {
         VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
         .memory = atlas_vbo.mem.handle,
         .offset = 0,
         .size = atlas_vbo.size
      };
      extern vk_context_t vk;
      vkFlushMappedMemoryRanges(vk.device, 1, &range);
   }
}
void vulkan_font_render(VkCommandBuffer cmd)
{
   VkDeviceSize offset = 0;
   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe);
   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe_layout, 0, 1, &atlas_desc, 0, NULL);

   vkCmdBindVertexBuffers(cmd, 0, 1, &atlas_vbo.handle, &offset);
   {
      VkMappedMemoryRange range =
      {
         VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
         .memory = atlas_g_buffer.mem.handle,
         .offset = 0,
         .size = sizeof(font_shader_storage_t)
      };
      extern vk_context_t vk;
      vkFlushMappedMemoryRanges(vk.device, 1, &range);
   }

   vkCmdDraw(cmd, atlas_vbo.size / sizeof(font_vertex_t), 1, 0, 0);
//   {
//      VkMappedMemoryRange range =
//      {
//         VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
//         .memory = atlas_g_buffer.mem.handle,
//         .offset = 0,
//         .size = sizeof(font_shader_storage_t)
//      };
//      extern vk_context_t vk;
//      vkInvalidateMappedMemoryRanges(vk.device, 1, &range);
//   }
//   printf("(float*)atlas_g_buffer.mem.ptr : %i\n", *(int*)atlas_g_buffer.mem.ptr);
}
