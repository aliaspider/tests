
#include <ctype.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "common.h"
#include "vulkan_common.h"
#include "font.h"
#include "video.h"

#define VK_ATLAS_WIDTH  512
#define VK_ATLAS_HEIGHT 512
static vk_texture_t atlas;
static VkDescriptorSet atlas_desc;
static FT_Library ftlib;
static vk_buffer_t atlas_vbo;
static vk_buffer_t atlas_ubo;
static vk_buffer_t atlas_g_buffer;
static VkPipeline pipe;
static VkPipelineLayout pipe_layout;
static int line_height;
static int ascender;
static int max_advance;

typedef struct
{
   uint8_t id;
   uint8_t x;
   uint8_t y;
   uint8_t w;
   uint8_t h;
   uint8_t advance;
} glyph_t;


typedef struct
{
   int last_id;
   int pos_x;
   int pos_y;
} font_shader_storage_t;

typedef struct
{
   float viewport_width;
   float viewport_height;
   float texture_width;
   float texture_height;
   struct
   {
      float x;
      float y;
      float w;
      float h;
   } glyph_metrics[256];

   float advance[256];
} font_uniforms_t;

typedef struct
{
   uint8_t id;
   struct
   {
      uint8_t r;
      uint8_t g;
      uint8_t b;
   } color;
   struct
   {
      float x;
      float y;
   } position;
} font_vertex_t;

typedef struct atlas_slot
{
   unsigned charcode;
   unsigned last_used;
   struct atlas_slot *next;
} atlas_slot_t;

static atlas_slot_t atlas_slots[256];
static atlas_slot_t *uc_map[256];
static unsigned usage_counter;

FT_Face ftface;

static int vulkan_font_get_new_slot(void)
{
   int i, map_id;
   unsigned oldest = 0;

   for (i = 1; i < 256; i++)
      if ((usage_counter - atlas_slots[i].last_used) >
         (usage_counter - atlas_slots[oldest].last_used))
         oldest = i;

   map_id = atlas_slots[oldest].charcode & 0xFF;

   if (uc_map[map_id] == &atlas_slots[oldest])
      uc_map[map_id] = atlas_slots[oldest].next;
   else if (uc_map[map_id])
   {
      atlas_slot_t *ptr = uc_map[map_id];

      while (ptr->next && ptr->next != &atlas_slots[oldest])
         ptr = ptr->next;

      ptr->next = atlas_slots[oldest].next;
   }

   return oldest;
}

static int vulkan_font_get_glyph_id(uint32_t charcode)
{
   unsigned map_id = charcode & 0xFF;

   {
      atlas_slot_t *atlas_slot = uc_map[map_id];

      while (atlas_slot)
      {
         if (atlas_slot->charcode == charcode)
         {
            atlas_slot->last_used = usage_counter++;
            return atlas_slot - atlas_slots;
         }

         atlas_slot = atlas_slot->next;
      }
   }

   int id = vulkan_font_get_new_slot();
   atlas_slots[id].charcode   = charcode;
   atlas_slots[id].next       = uc_map[map_id];
   uc_map[map_id] = &atlas_slots[id];

   uint8_t *dst = atlas.staging.mem.u8 + atlas.staging.mem_layout.offset +
      (id & 0xF) * (VK_ATLAS_WIDTH / 16) + (((id >> 4) * (VK_ATLAS_HEIGHT / 16))) * VK_ATLAS_WIDTH;
   int row;
#if 1
   FT_Load_Char(ftface, charcode, FT_LOAD_RENDER);
//   FT_Render_Glyph(ftface->glyph, FT_RENDER_MODE_NORMAL);

   uint8_t *src = ftface->glyph->bitmap.buffer;

   for (row = 0; row < ftface->glyph->bitmap.rows; row++)
   {
      memcpy(dst, src, ftface->glyph->bitmap.width);
      src += ftface->glyph->bitmap.pitch;
      dst += atlas.staging.mem_layout.rowPitch;
   }

#else
   FT_Load_Char(ftface, charcode, FT_LOAD_RENDER | FT_LOAD_MONOCHROME);
//      FT_Render_Glyph(ftface->glyph, FT_RENDER_MODE_MONO);

   uint8_t *src = ftface->glyph->bitmap.buffer;

   for (row = 0; row < ftface->glyph->bitmap.rows; row++)
   {
      int col;

      for (col = 0; col < ftface->glyph->bitmap.width; col++)
      {
         if (src[col >> 3] & (0x80 >> (col & 0x7)))
            dst[col] = 255;
      }

      src += ftface->glyph->bitmap.pitch;
      dst += atlas.staging.mem_layout.rowPitch;
   }

#endif

   atlas.dirty = true;

   ((font_uniforms_t *)atlas_ubo.mem.ptr)->glyph_metrics[id].x = ftface->glyph->metrics.horiBearingX >> 6;
   ((font_uniforms_t *)atlas_ubo.mem.ptr)->glyph_metrics[id].y = -ftface->glyph->metrics.horiBearingY >> 6;
   ((font_uniforms_t *)atlas_ubo.mem.ptr)->glyph_metrics[id].w = ftface->glyph->metrics.width >> 6;
   ((font_uniforms_t *)atlas_ubo.mem.ptr)->glyph_metrics[id].h = ftface->glyph->metrics.height >> 6;
   ((font_uniforms_t *)atlas_ubo.mem.ptr)->advance[id] = ftface->glyph->metrics.horiAdvance >> 6;

   atlas_ubo.dirty = true;
   atlas_slots[id].last_used = usage_counter++;
   return id;
}


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
      buffer_init_info_t info =
      {
         .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
         .req_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
         .size = sizeof(font_uniforms_t),
      };
      buffer_init(device, memory_types, &info, &atlas_ubo);
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
      const VkDescriptorBufferInfo uniform_buffer_info =
      {
         .buffer = atlas_ubo.handle,
         .offset = 0,
         .range = sizeof(font_uniforms_t)
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
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .pBufferInfo = &uniform_buffer_info
         },
         {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = atlas_desc,
            .dstBinding = 1,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = &image_info
         },
         {
            VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = atlas_desc,
            .dstBinding = 2,
            .dstArrayElement = 0,
            .descriptorCount = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .pBufferInfo = &buffer_info
         },
      };
      vkUpdateDescriptorSets(device, countof(write_set), write_set, 0, NULL);
   }


   memset(atlas.staging.mem.u8 + atlas.staging.mem_layout.offset, 0x0,
      atlas.staging.mem_layout.size - atlas.staging.mem_layout.offset);

   atlas.dirty = true;

   {
//      const char *font_path = "/usr/share/fonts/TTF/DejaVuSansMono.ttf";
//      const char *font_path = "/usr/share/fonts/TTF/NotoSerif-Regular.ttf";
      const char *font_path = "/usr/share/fonts/TTF/HanaMinA.ttf";
//      const char* font_path = "/usr/share/fonts/TTF/LiberationMono-Regular.ttf";
//      const char* font_path = "/usr/share/fonts/75dpi/charR12.pcf.gz";
//      const char* font_path = "/usr/share/fonts/WindowsFonts/cour.ttf";


      FT_UInt font_size = 26;

      FT_Init_FreeType(&ftlib);
      FT_New_Face(ftlib, font_path, 0, &ftface);

      FT_Select_Charmap(ftface, FT_ENCODING_UNICODE);
      FT_Set_Pixel_Sizes(ftface, 0, font_size);

      line_height = ftface->size->metrics.height >> 6;
      ascender = ftface->size->metrics.ascender >> 6;
      max_advance = ftface->size->metrics.max_advance >> 6;
//      atlas_ubo.dirty = true;

   }

   ((font_uniforms_t *)atlas_ubo.mem.ptr)->viewport_width = video.screen.width;
   ((font_uniforms_t *)atlas_ubo.mem.ptr)->viewport_height = video.screen.height;
   ((font_uniforms_t *)atlas_ubo.mem.ptr)->texture_width = atlas.width;
   ((font_uniforms_t *)atlas_ubo.mem.ptr)->texture_height = atlas.height;
   atlas_ubo.dirty = true;

//   device_memory_flush(device, &atlas_ubo.mem);
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
               .dstAlphaBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
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

void vulkan_font_destroy(VkDevice device)
{
   FT_Done_Face(ftface);
   FT_Done_FreeType(ftlib);
   buffer_free(device, &atlas_vbo);
   texture_free(device, &atlas);
   vkDestroyPipelineLayout(device, pipe_layout, NULL);
   vkDestroyPipeline(device, pipe, NULL);
   pipe_layout = VK_NULL_HANDLE;
   pipe = VK_NULL_HANDLE;

//   vkFreeDescriptorSets(device, pool, 1, &atlas_desc);
   atlas_desc = VK_NULL_HANDLE;

}

void vulkan_font_draw_text(const char *text, int x, int y)
{
   const unsigned char *in = (const unsigned char*)text;

   font_vertex_t *out = (font_vertex_t *)(atlas_vbo.mem.u8 + atlas_vbo.size);
   font_vertex_t vertex;
   vertex.color.r = 0;
   vertex.color.g = 0;
   vertex.color.b = 0;
   vertex.position.x = x;
   vertex.position.y = y + ascender;
   vertex.id = 'p';

   while (*in)
   {
      uint32_t charcode = *(in++);

      if (charcode == '\n')
      {
         vertex.position.x = 0;
         vertex.position.y += line_height;
         continue;
      }

      if((charcode & 0xC0) == 0xC0)
      {
         int marker = charcode & 0xE0;
         charcode = ((charcode & ~0xE0) << 6) | (*(in++) & ~0xC0);
         if(marker == 0xE0)
         {
            charcode = (charcode << 6) | (*(in++) & ~0xC0);
            if(charcode & 1 << (4 + 6 + 6))
               charcode = (charcode & ~(1 << (4 + 6 + 6)) << 6) | (*(in++) & ~0xC0);
         }
      }

      *out = vertex;
      int id = vulkan_font_get_glyph_id(charcode);
      vertex.position.x += ((font_uniforms_t *)atlas_ubo.mem.ptr)->advance[id];

      if ((vertex.position.x + max_advance) > video.screen.width)
      {
         vertex.position.x = 0;
         vertex.position.y += line_height;
      }

      (out++)->id = id;
   }

   atlas_vbo.size = (uint8_t *)out - atlas_vbo.mem.u8;


}

void vulkan_font_update_assets(VkCommandBuffer cmd)
{
   atlas_vbo.size = 0;

//   vulkan_font_draw_text("Backward compatibility: Backwards compatibility with ASCII and the enormous "
//                         "amount of software designed to process ASCII-encoded text was the main driving "
//                         "force behind the design of UTF-8. In UTF-8, single bytes with values in the range "
//                         "of 0 to 127 map directly to Unicode code points in the ASCII range. Single bytes "
//                         "in this range represent characters, as they do in ASCII.\n\nMoreover, 7-bit bytes "
//                         "(bytes where the most significant bit is 0) never appear in a multi-byte sequence, "
//                         "and no valid multi-byte sequence decodes to an ASCII code-point. A sequence of 7-bit "
//                         "bytes is both valid ASCII and valid UTF-8, and under either interpretation represents "
//                         "the same sequence of characters.\n\nTherefore, the 7-bit bytes in a UTF-8 stream represent "
//                         "all and only the ASCII characters in the stream. Thus, many text processors, parsers, "
//                         "protocols, file formats, text display programs etc., which use ASCII characters for "
//                         "formatting and control purposes will continue to work as intended by treating the UTF-8 "
//                         "byte stream as a sequence of single-byte characters, without decoding the multi-byte sequences. "
//                         "ASCII characters on which the processing turns, such as punctuation, whitespace, and control "
//                         "characters will never be encoded as multi-byte sequences. It is therefore safe for such "
//                         "processors to simply ignore or pass-through the multi-byte sequences, without decoding them. "
//                         "For example, ASCII whitespace may be used to tokenize a UTF-8 stream into words; "
//                         "ASCII line-feeds may be used to split a UTF-8 stream into lines; and ASCII NUL ", 0, 0);

   vulkan_font_draw_text("北海道の有名なかん光地、知床半島で、黒いキツネがさつえいされました。"
      "地元斜里町の町立知床博物館が、タヌキをかんさつするためにおいていた自動さつえいカメラがき重なすがたをとらえました＝"
      "写真・同館ていきょう。同館の学芸員も「はじめて見た」とおどろいています。 "
      "黒い毛皮のために昔、ゆ入したキツネの子そんとも言われてますが、はっきりしません。"
      "北海道の先住みん族、アイヌのみん話にも黒いキツネが登場し、神せいな生き物とされているそうです。", 0, 0);
//   vulkan_font_draw_text("gl_Position.xy = pos + 2.0 * vec2(0.0, glyph_metrics[c].w) / vp_size;", 0, 32);

//   vulkan_font_draw_text("test 3", 40, 220);
   if (atlas.dirty)
      texture_update(cmd, &atlas);

   if (atlas_vbo.dirty)
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
      atlas_vbo.dirty = false;
   }

   if (atlas_ubo.dirty)
   {
      VkMappedMemoryRange range =
      {
         VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
         .memory = atlas_ubo.mem.handle,
         .offset = 0,
         .size = atlas_ubo.size
      };
      extern vk_context_t vk;
      vkFlushMappedMemoryRanges(vk.device, 1, &range);
      atlas_ubo.dirty = false;
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
