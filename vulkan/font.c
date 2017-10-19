
#include <ctype.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "common.h"
#include "vulkan_common.h"
#include "font.h"
#include "video.h"

#define VK_ATLAS_WIDTH  512
#define VK_ATLAS_HEIGHT 512

typedef struct
{
   int dummy;
} font_shader_storage_t;

typedef union
{
   struct
   {
      float r;
      float g;
   };
   struct
   {
      float x;
      float y;
   };
   struct
   {
      float width;
      float height;
   };
} vec2;

typedef union
{
   struct
   {
      float r;
      float g;
      float b;
      float a;
   };
   struct
   {
      float x;
      float y;
      union
      {
         struct
         {
            float z;
            float w;
         };
         struct
         {
            float width;
            float height;
         };
      };
   };
} vec4;

typedef struct
{
   vec2 vp_size;
   vec2 tex_size;
   vec4 glyph_metrics[256];

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


static struct
{
   FT_Library ftlib;
   FT_Face ftface;
   int line_height;
   int ascender;
   int max_advance;
   struct
   {
      vk_render_t render;
      int slot_width;
      int slot_height;
      atlas_slot_t slots[256];
      atlas_slot_t *uc_map[256];
      unsigned usage_counter;
   } atlas;
} font;

static int vulkan_font_get_new_slot(void)
{
   unsigned oldest = 0;

   int i;

   for (i = 1; i < 256; i++)
      if ((font.atlas.usage_counter - font.atlas.slots[i].last_used) >
         (font.atlas.usage_counter - font.atlas.slots[oldest].last_used))
         oldest = i;

   int map_id = font.atlas.slots[oldest].charcode & 0xFF;

   if (font.atlas.uc_map[map_id] == &font.atlas.slots[oldest])
      font.atlas.uc_map[map_id] = font.atlas.slots[oldest].next;
   else if (font.atlas.uc_map[map_id])
   {
      atlas_slot_t *ptr = font.atlas.uc_map[map_id];

      while (ptr->next && ptr->next != &font.atlas.slots[oldest])
         ptr = ptr->next;

      ptr->next = font.atlas.slots[oldest].next;
   }

   return oldest;
}

static int vulkan_font_get_glyph_id(uint32_t charcode)
{
   unsigned map_id = charcode & 0xFF;

   {
      atlas_slot_t *atlas_slot = font.atlas.uc_map[map_id];

      while (atlas_slot)
      {
         if (atlas_slot->charcode == charcode)
         {
            atlas_slot->last_used = font.atlas.usage_counter++;
            return atlas_slot - font.atlas.slots;
         }

         atlas_slot = atlas_slot->next;
      }
   }

   int id = vulkan_font_get_new_slot();
   font.atlas.slots[id].charcode   = charcode;
   font.atlas.slots[id].next       = font.atlas.uc_map[map_id];
   font.atlas.uc_map[map_id] = &font.atlas.slots[id];

   {
      uint8_t *dst = font.atlas.render.texture.staging.mem.u8 + font.atlas.render.texture.staging.mem_layout.offset +
         (id & 0xF) * font.atlas.slot_width + (((id >> 4) * font.atlas.slot_height)) *
         font.atlas.render.texture.staging.mem_layout.rowPitch;
      int row;
#if 1
      FT_Load_Char(font.ftface, charcode, FT_LOAD_RENDER);
//   FT_Render_Glyph(ftface->glyph, FT_RENDER_MODE_NORMAL);

      uint8_t *src = font.ftface->glyph->bitmap.buffer;

      assert((dst - font.atlas.render.texture.staging.mem.u8 + font.atlas.render.texture.staging.mem_layout.rowPitch *
            (font.ftface->glyph->bitmap.rows + 1) < font.atlas.render.texture.staging.mem_layout.size));

      for (row = 0; row < font.ftface->glyph->bitmap.rows; row++)
      {
         memcpy(dst, src, font.ftface->glyph->bitmap.width);
         src += font.ftface->glyph->bitmap.pitch;
         dst += font.atlas.render.texture.staging.mem_layout.rowPitch;
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
      font.atlas.render.texture.dirty = true;
   }

   {
      font_uniforms_t *uniforms = (font_uniforms_t *)font.atlas.render.ubo.mem.ptr;
      uniforms->glyph_metrics[id].x = font.ftface->glyph->metrics.horiBearingX >> 6;
      uniforms->glyph_metrics[id].y = -font.ftface->glyph->metrics.horiBearingY >> 6;
      uniforms->glyph_metrics[id].width = font.ftface->glyph->metrics.width >> 6;
      uniforms->glyph_metrics[id].height = font.ftface->glyph->metrics.height >> 6;
      uniforms->advance[id] = font.ftface->glyph->metrics.horiAdvance >> 6;
      font.atlas.render.ubo.dirty = true;
   }

   font.atlas.slots[id].last_used = font.atlas.usage_counter++;
   return id;
}


void vulkan_font_init(vk_context_t *vk, vk_render_context_t *vk_render)
{

   {
//      const char *font_path = "/usr/share/fonts/TTF/DejaVuSansMono.ttf";
//      const char *font_path = "/usr/share/fonts/TTF/NotoSerif-Regular.ttf";
      const char *font_path = "/usr/share/fonts/TTF/HanaMinA.ttf";
//      const char* font_path = "/usr/share/fonts/TTF/LiberationMono-Regular.ttf";
//      const char* font_path = "/usr/share/fonts/75dpi/charR12.pcf.gz";
//      const char* font_path = "/usr/share/fonts/WindowsFonts/cour.ttf";

      FT_UInt font_size = 26;      
      FT_Init_FreeType(&font.ftlib);
      FT_New_Face(font.ftlib, font_path, 0, &font.ftface);
      FT_Select_Charmap(font.ftface, FT_ENCODING_UNICODE);
      FT_Set_Pixel_Sizes(font.ftface, 0, font_size);
   }

   font.line_height = font.ftface->size->metrics.height >> 6;
   font.ascender = font.ftface->size->metrics.ascender >> 6;
   font.max_advance = font.ftface->size->metrics.max_advance >> 6;
   font.atlas.slot_width = font.max_advance;
   font.atlas.slot_height = font.line_height;

   font.atlas.render.texture.width = font.atlas.slot_width << 4;
   font.atlas.render.texture.height = font.atlas.slot_height << 4;
   font.atlas.render.texture.format = VK_FORMAT_R8_UNORM;
   font.atlas.render.ssbo.info.range = sizeof(font_shader_storage_t);
   font.atlas.render.ssbo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   font.atlas.render.ssbo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
   font.atlas.render.ubo.info.range = sizeof(font_uniforms_t);
   font.atlas.render.ubo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   font.atlas.render.ubo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;

   vk_render_init(vk, vk_render, &font.atlas.render);


   memset(font.atlas.render.texture.staging.mem.u8 + font.atlas.render.texture.staging.mem_layout.offset, 0x0,
      font.atlas.render.texture.staging.mem_layout.size - font.atlas.render.texture.staging.mem_layout.offset);

   device_memory_flush(vk->device, &font.atlas.render.texture.staging.mem);
   font.atlas.render.texture.dirty = true;

   font_uniforms_t *uniforms = (font_uniforms_t *)font.atlas.render.ubo.mem.ptr;
   uniforms->vp_size.width = video.screen.width;
   uniforms->vp_size.height = video.screen.height;
   uniforms->tex_size.width = font.atlas.render.texture.width;
   uniforms->tex_size.height = font.atlas.render.texture.height;
   font.atlas.render.ubo.dirty = true;

   font.atlas.render.vbo.info.range = 4096 * sizeof(font_vertex_t);
   font.atlas.render.vbo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   font.atlas.render.vbo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
   buffer_init(vk->device, vk->memoryTypes, NULL, &font.atlas.render.vbo);
   font.atlas.render.vbo.info.range = 0;


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
         .setLayoutCount = 1, &vk_render->descriptor_set_layout,
#if 0
         .pushConstantRangeCount = countof(ranges), ranges
#endif
      };

      vkCreatePipelineLayout(vk->device, &info, NULL, &font.atlas.render.layout);
   }

   {
      VkShaderModule vertex_shader;
      {
         const uint32_t code [] =
#include "font.vert.inc"
            ;
         const VkShaderModuleCreateInfo info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = sizeof(code), .pCode = code};
         vkCreateShaderModule(vk->device, &info, NULL, &vertex_shader);
      }

      VkShaderModule fragment_shader;
      {
         const uint32_t code [] =
#include "font.frag.inc"
            ;
         const VkShaderModuleCreateInfo info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = sizeof(code), .pCode = code};
         vkCreateShaderModule(vk->device, &info, NULL, &fragment_shader);
      }

      VkShaderModule geometry_shader;
      {
         const uint32_t code [] =
#include "font.geom.inc"
            ;
         const VkShaderModuleCreateInfo info = {VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO, .codeSize = sizeof(code), .pCode = code};
         vkCreateShaderModule(vk->device, &info, NULL, &geometry_shader);
      }

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

      const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {0, 0, VK_FORMAT_R8_UINT, offsetof(font_vertex_t, id)},
         {1, 0, VK_FORMAT_R8G8B8_UNORM, offsetof(font_vertex_t, color)},
         {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(font_vertex_t, position)}
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
         .viewportCount = 1, &vk_render->viewport, .scissorCount = 1, &vk_render->scissor
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
         .layout = font.atlas.render.layout,
         .renderPass = vk_render->renderpass,
         .subpass = 0
      };
      vkCreateGraphicsPipelines(vk->device, VK_NULL_HANDLE, 1, &info, NULL, &font.atlas.render.pipe);
      vkDestroyShaderModule(vk->device, vertex_shader, NULL);
      vkDestroyShaderModule(vk->device, fragment_shader, NULL);
      vkDestroyShaderModule(vk->device, geometry_shader, NULL);
   }
}

void vulkan_font_destroy(VkDevice device)
{
   FT_Done_Face(font.ftface);
   FT_Done_FreeType(font.ftlib);
   buffer_free(device, &font.atlas.render.vbo);
   texture_free(device, &font.atlas.render.texture);
   vkDestroyPipelineLayout(device, font.atlas.render.layout, NULL);
   vkDestroyPipeline(device, font.atlas.render.pipe, NULL);
   memset(&font, 0, sizeof(font));
}

void vulkan_font_draw_text(const char *text, int x, int y)
{
   const unsigned char *in = (const unsigned char *)text;
   font_vertex_t *last_space = NULL;

   font_vertex_t *out = (font_vertex_t *)(font.atlas.render.vbo.mem.u8 + font.atlas.render.vbo.info.range);
   font_vertex_t vertex;
   vertex.color.r = 0;
   vertex.color.g = 0;
   vertex.color.b = 0;
   vertex.position.x = x;
   vertex.position.y = y + font.ascender;
   vertex.id = 'p';

   while (*in)
   {
      uint32_t charcode = *(in++);

      if (charcode == '\n')
      {
         vertex.position.x = 0;
         vertex.position.y += font.line_height;
         continue;
      }

      if (charcode == ' ')
         last_space = out;

      if ((charcode & 0xC0) == 0xC0)
      {
         int marker = charcode & 0xE0;
         charcode = ((charcode & ~0xE0) << 6) | (*(in++) & ~0xC0);

         if (marker == 0xE0)
         {
            charcode = (charcode << 6) | (*(in++) & ~0xC0);

            if (charcode & 0x10000)
               charcode = (charcode & 0xFFFF << 6) | (*(in++) & ~0xC0);
         }
      }

      int id = vulkan_font_get_glyph_id(charcode);

      if ((vertex.position.x + ((font_uniforms_t *)font.atlas.render.ubo.mem.ptr)->advance[id]) > video.screen.width)
//      if ((vertex.position.x + font.max_advance) > video.screen.width)
      {
         vertex.position.x = 0;
         vertex.position.y += font.line_height;

         if (last_space && (last_space + (video.screen.width / (2 * font.max_advance))) > out)
         {
            font_vertex_t *ptr = last_space + 1;

            while (ptr < out)
            {
               ptr->position.x = vertex.position.x;
               ptr->position.y = vertex.position.y;
               vertex.position.x += ((font_uniforms_t *)font.atlas.render.ubo.mem.ptr)->advance[ptr->id];
               ptr++;
            }

            last_space = NULL;
         }
      }

      *out = vertex;
      vertex.position.x += ((font_uniforms_t *)font.atlas.render.ubo.mem.ptr)->advance[id];

      (out++)->id = id;
   }

   font.atlas.render.vbo.info.range = (uint8_t *)out - font.atlas.render.vbo.mem.u8;


}

void vulkan_font_update_assets(VkDevice device, VkCommandBuffer cmd)
{
   font.atlas.render.vbo.info.range = 0;

//   vulkan_font_draw_text("Backward compatibility: Backwards compatibility with ASCII and the enormous "
//      "amount of software designed to process ASCII-encoded text was the main driving "
//      "force behind the design of UTF-8. In UTF-8, single bytes with values in the range "
//      "of 0 to 127 map directly to Unicode code points in the ASCII range. Single bytes "
//      "in this range represent characters, as they do in ASCII.\n\nMoreover, 7-bit bytes "
//      "(bytes where the most significant bit is 0) never appear in a multi-byte sequence, "
//      "and no valid multi-byte sequence decodes to an ASCII code-point. A sequence of 7-bit "
//      "bytes is both valid ASCII and valid UTF-8, and under either interpretation represents "
//      "the same sequence of characters.\n\nTherefore, the 7-bit bytes in a UTF-8 stream represent "
//      "all and only the ASCII characters in the stream. Thus, many text processors, parsers, "
//      "protocols, file formats, text display programs etc., which use ASCII characters for "
//      "formatting and control purposes will continue to work as intended by treating the UTF-8 "
//      "byte stream as a sequence of single-byte characters, without decoding the multi-byte sequences. "
//      "ASCII characters on which the processing turns, such as punctuation, whitespace, and control "
//      "characters will never be encoded as multi-byte sequences. It is therefore safe for such "
//      "processors to simply ignore or pass-through the multi-byte sequences, without decoding them. "
//      "For example, ASCII whitespace may be used to tokenize a UTF-8 stream into words; "
//      "ASCII line-feeds may be used to split a UTF-8 stream into lines; and ASCII NUL ", 0, 0);

   vulkan_font_draw_text("北海道の有名なかん光地、知床半島で、黒いキツネがさつえいされました。"
      "地元斜里町の町立知床博物館が、タヌキをかんさつするためにおいていた自動さつえいカメラがき重なすがたをとらえました＝"
      "写真・同館ていきょう。同館の学芸員も「はじめて見た」とおどろいています。"
      "黒い毛皮のために昔、ゆ入したキツネの子そんとも言われてますが、はっきりしません。"
      "北海道の先住みん族、アイヌのみん話にも黒いキツネが登場し、神せいな生き物とされているそうです。",
      0, 0);

//   vulkan_font_draw_text("gl_Position.xy = pos + 2.0 * vec2(0.0, glyph_metrics[c].w) / vp_size;", 0, 32);

//   vulkan_font_draw_text("test 3", 40, 220);
   if (font.atlas.render.texture.dirty)
      texture_update(cmd, &font.atlas.render.texture);

   if (font.atlas.render.vbo.dirty)
      buffer_flush(device, &font.atlas.render.vbo);

   if (font.atlas.render.ubo.dirty)
      buffer_flush(device, &font.atlas.render.ubo);

   if (font.atlas.render.ssbo.dirty)
      buffer_flush(device, &font.atlas.render.ssbo);
}
void vulkan_font_render(VkCommandBuffer cmd)
{
   if (!font.atlas.render.vbo.info.range)
      return;

   VkDeviceSize offset = 0;
   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, font.atlas.render.pipe);
   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, font.atlas.render.layout, 0, 1, &font.atlas.render.desc, 0, NULL);
   vkCmdBindVertexBuffers(cmd, 0, 1, &font.atlas.render.vbo.info.buffer, &offset);
   vkCmdDraw(cmd, font.atlas.render.vbo.info.range / sizeof(font_vertex_t), 1, 0, 0);
//   {
//      VkMappedMemoryRange range =
//      {
//         VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
//         .memory = atlas_ssbo.mem.handle,
//         .offset = 0,
//         .size = sizeof(font_shader_storage_t)
//      };
//      extern vk_context_t vk;
//      vkInvalidateMappedMemoryRanges(vk.device, 1, &range);
//   }
//   printf("(float*)atlas_ssbo.mem.ptr : %i\n", *(int*)atlas_ssbo.mem.ptr);
}
