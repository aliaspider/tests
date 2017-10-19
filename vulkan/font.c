
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

typedef struct
{
   vec2 vp_size;
   vec2 tex_size;
   vec4 glyph_metrics[256];
   float advance[256];
} font_uniforms_t;

typedef struct
{
   uint8_t slot_id;
   struct
   {
      uint8_t r, g, b;
   } color;
   struct
   {
      float x, y;
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
   vk_render_t render;
   struct
   {
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

static void ft_font_render_glyph(unsigned charcode, int slot_id)
{
   {
      int row;

      FT_Int32  load_flags = FT_LOAD_RENDER;
//      FT_Int32  load_flags = FT_LOAD_RENDER | FT_LOAD_MONOCHROME;
      FT_Load_Char(font.ftface, charcode, load_flags);

      uint8_t *src = font.ftface->glyph->bitmap.buffer;

      uint8_t *dst = font.render.texture.staging.mem.u8 + font.render.texture.staging.mem_layout.offset +
         (slot_id & 0xF) * font.atlas.slot_width + (((slot_id >> 4) * font.atlas.slot_height)) *
         font.render.texture.staging.mem_layout.rowPitch;

      assert((dst - font.render.texture.staging.mem.u8 + font.render.texture.staging.mem_layout.rowPitch *
            (font.ftface->glyph->bitmap.rows + 1) < font.render.texture.staging.mem_layout.size));

      for (row = 0; row < font.ftface->glyph->bitmap.rows; row++)
      {
         if (load_flags & FT_LOAD_MONOCHROME)
         {
            int col;

            for (col = 0; col < font.ftface->glyph->bitmap.width; col++)
            {
               if (src[col >> 3] & (0x80 >> (col & 0x7)))
                  dst[col] = 255;
            }
         }
         else
            memcpy(dst, src, font.ftface->glyph->bitmap.width);

         src += font.ftface->glyph->bitmap.pitch;
         dst += font.render.texture.staging.mem_layout.rowPitch;
      }

      font.render.texture.dirty = true;
   }

   {
      font_uniforms_t *uniforms = (font_uniforms_t *)font.render.ubo.mem.ptr;
      uniforms->glyph_metrics[slot_id].x = font.ftface->glyph->metrics.horiBearingX >> 6;
      uniforms->glyph_metrics[slot_id].y = -font.ftface->glyph->metrics.horiBearingY >> 6;
      uniforms->glyph_metrics[slot_id].width = font.ftface->glyph->metrics.width >> 6;
      uniforms->glyph_metrics[slot_id].height = font.ftface->glyph->metrics.height >> 6;
      uniforms->advance[slot_id] = font.ftface->glyph->metrics.horiAdvance >> 6;
      font.render.ubo.dirty = true;
   }
}

static int vulkan_font_get_slot_id(uint32_t charcode)
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

   int slot_id = vulkan_font_get_new_slot();
   font.atlas.slots[slot_id].charcode   = charcode;
   font.atlas.slots[slot_id].next       = font.atlas.uc_map[map_id];
   font.atlas.uc_map[map_id] = &font.atlas.slots[slot_id];

   ft_font_render_glyph(charcode, slot_id);

   font.atlas.slots[slot_id].last_used = font.atlas.usage_counter++;
   return slot_id;
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

   {
      const uint32_t vs_code [] =
#include "font.vert.inc"
         ;
      const uint32_t ps_code [] =
#include "font.frag.inc"
         ;
      const uint32_t gs_code [] =
#include "font.geom.inc"
         ;

      const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {0, 0, VK_FORMAT_R8_UINT, offsetof(font_vertex_t, slot_id)},
         {1, 0, VK_FORMAT_R8G8B8_UNORM, offsetof(font_vertex_t, color)},
         {2, 0, VK_FORMAT_R32G32_SFLOAT, offsetof(font_vertex_t, position)}
      };

      const VkPipelineColorBlendAttachmentState color_blend_attachement_state =
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

      const vk_pipeline_init_info_t info =
      {
         .shaders.vs.code = vs_code,
         .shaders.vs.code_size = sizeof(vs_code),
         .shaders.ps.code = ps_code,
         .shaders.ps.code_size = sizeof(ps_code),
         .shaders.gs.code = gs_code,
         .shaders.gs.code_size = sizeof(gs_code),
         .vertex_stride = sizeof(font_vertex_t),
         .attrib_count = countof(attrib_desc),
         .attrib_desc = attrib_desc,
         .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
         .color_blend_attachement_state = &color_blend_attachement_state,
      };

      font.render.texture.width = font.atlas.slot_width << 4;
      font.render.texture.height = font.atlas.slot_height << 4;
      font.render.texture.format = VK_FORMAT_R8_UNORM;

      font.render.ssbo.info.range = sizeof(font_shader_storage_t);
      font.render.ubo.info.range = sizeof(font_uniforms_t);
      font.render.vbo.info.range = 4096 * sizeof(font_vertex_t);

      vk_render_init(vk, vk_render, &info, &font.render);
   }

   memset(font.render.texture.staging.mem.u8 + font.render.texture.staging.mem_layout.offset, 0x0,
      font.render.texture.staging.mem_layout.size - font.render.texture.staging.mem_layout.offset);

   font.render.texture.dirty = true;

   font_uniforms_t *uniforms = (font_uniforms_t *)font.render.ubo.mem.ptr;
   uniforms->vp_size.width = video.screen.width;
   uniforms->vp_size.height = video.screen.height;
   uniforms->tex_size.width = font.render.texture.width;
   uniforms->tex_size.height = font.render.texture.height;
   font.render.ubo.dirty = true;

   font.render.vbo.info.range = 0;
}

void vulkan_font_destroy(VkDevice device)
{
   FT_Done_Face(font.ftface);
   FT_Done_FreeType(font.ftlib);
   vk_render_destroy(device, &font.render);
}

void vulkan_font_draw_text(const char *text, int x, int y)
{
   const unsigned char *in = (const unsigned char *)text;
   font_vertex_t *last_space = NULL;

   font_vertex_t *out = (font_vertex_t *)(font.render.vbo.mem.u8 + font.render.vbo.info.range);
   font_vertex_t vertex;
   vertex.color.r = 0;
   vertex.color.g = 0;
   vertex.color.b = 0;
   vertex.position.x = x;
   vertex.position.y = y + font.ascender;
   vertex.slot_id = 'p';

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

      int slot_id = vulkan_font_get_slot_id(charcode);

      if ((vertex.position.x + ((font_uniforms_t *)font.render.ubo.mem.ptr)->advance[slot_id]) > video.screen.width)
//      if ((vertex.position.x + font.max_advance) > video.screen.width)
      {
         vertex.position.y += font.line_height;

         if (last_space && (last_space + 1 < out) && (last_space + (video.screen.width / (2 * font.max_advance))) > out)
         {
            font_vertex_t *ptr = last_space + 1;
            int old_x = ptr->position.x;

            vertex.position.x -= old_x;

            while (ptr < out)
            {
               ptr->position.x -= old_x;
               ptr->position.y = vertex.position.y;
               ptr++;
            }

            last_space = NULL;
         }
         else
            vertex.position.x = 0;
      }

      *out = vertex;
      vertex.position.x += ((font_uniforms_t *)font.render.ubo.mem.ptr)->advance[slot_id];

      (out++)->slot_id = slot_id;
   }

   font.render.vbo.info.range = (uint8_t *)out - font.render.vbo.mem.u8;


}

void vulkan_font_update_assets(VkDevice device, VkCommandBuffer cmd)
{
   font.render.vbo.info.range = 0;

   vulkan_font_draw_text("Backward compatibility: Backwards compatibility with ASCII and the enormous "
      "amount of software designed to process ASCII-encoded text was the main driving "
      "force behind the design of UTF-8. In UTF-8, single bytes with values in the range "
      "of 0 to 127 map directly to Unicode code points in the ASCII range. Single bytes "
      "in this range represent characters, as they do in ASCII.\n\nMoreover, 7-bit bytes "
      "(bytes where the most significant bit is 0) never appear in a multi-byte sequence, "
      "and no valid multi-byte sequence decodes to an ASCII code-point. A sequence of 7-bit "
      "bytes is both valid ASCII and valid UTF-8, and under either interpretation represents "
      "the same sequence of characters.\n\nTherefore, the 7-bit bytes in a UTF-8 stream represent "
      "all and only the ASCII characters in the stream. Thus, many text processors, parsers, "
      "protocols, file formats, text display programs etc., which use ASCII characters for "
      "formatting and control purposes will continue to work as intended by treating the UTF-8 "
      "byte stream as a sequence of single-byte characters, without decoding the multi-byte sequences. "
      "ASCII characters on which the processing turns, such as punctuation, whitespace, and control "
      "characters will never be encoded as multi-byte sequences. It is therefore safe for such "
      "processors to simply ignore or pass-through the multi-byte sequences, without decoding them. "
      "For example, ASCII whitespace may be used to tokenize a UTF-8 stream into words; "
      "ASCII line-feeds may be used to split a UTF-8 stream into lines; and ASCII NUL ", 0, 0);

//   vulkan_font_draw_text("北海道の有名なかん光地、知床半島で、黒いキツネがさつえいされました。"
//      "地元斜里町の町立知床博物館が、タヌキをかんさつするためにおいていた自動さつえいカメラがき重なすがたをとらえました＝"
//      "写真・同館ていきょう。同館の学芸員も「はじめて見た」とおどろいています。"
//      "黒い毛皮のために昔、ゆ入したキツネの子そんとも言われてますが、はっきりしません。"
//      "北海道の先住みん族、アイヌのみん話にも黒いキツネが登場し、神せいな生き物とされているそうです。",
//      0, 0);

//   vulkan_font_draw_text("gl_Position.xy = pos + 2.0 * vec2(0.0, glyph_metrics[c].w) / vp_size;", 0, 32);

//   vulkan_font_draw_text("test 3", 40, 220);
   if (font.render.texture.dirty)
      texture_update(device, cmd, &font.render.texture);

   if (font.render.vbo.dirty)
      buffer_flush(device, &font.render.vbo);

   if (font.render.ubo.dirty)
      buffer_flush(device, &font.render.ubo);

   if (font.render.ssbo.dirty)
      buffer_flush(device, &font.render.ssbo);
}

void vulkan_font_render(VkCommandBuffer cmd)
{
   if ((font.render.vbo.info.range - font.render.vbo.info.offset) == 0)
      return;

   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, font.render.pipe);
   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, font.render.pipeline_layout, 0, 1, &font.render.desc, 0,
      NULL);
   vkCmdBindVertexBuffers(cmd, 0, 1, &font.render.vbo.info.buffer, &font.render.vbo.info.offset);
   vkCmdDraw(cmd, (font.render.vbo.info.range - font.render.vbo.info.offset) / sizeof(font_vertex_t), 1, 0, 0);
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
