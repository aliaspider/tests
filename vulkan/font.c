
#include <ctype.h>
#include <stdbool.h>
#include <stdio.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include "common.h"
#include "vulkan_common.h"
#include "font.h"
#include "video.h"
#include "input.h"

#define VK_ATLAS_WIDTH  512
#define VK_ATLAS_HEIGHT 512

typedef struct
{
   int dummy;
} font_shader_storage_t;

typedef struct
{
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
   float line_height;
   int ascender;
   int max_advance;
   vk_pipeline_t p;
   bool monochrome;
   struct
   {
      int slot_width;
      int slot_height;
      atlas_slot_t slot_map[256];
      atlas_slot_t *uc_map[256];
      unsigned usage_counter;
   } atlas;
} font;

void vulkan_font_init(vk_context_t *vk)
{

   {
      const char *font_path = "/usr/share/fonts/TTF/DejaVuSansMono.ttf";
//      const char *font_path = "/usr/share/fonts/TTF/NotoSerif-Regular.ttf";
//      const char *font_path = "/usr/share/fonts/TTF/HanaMinA.ttf";
//      const char* font_path = "/usr/share/fonts/TTF/LiberationMono-Regular.ttf";
//      const char* font_path = "/usr/share/fonts/75dpi/courR18.pcf.gz";
//      const char* font_path = "/usr/share/fonts/WindowsFonts/cour.ttf";

      FT_UInt font_size = 18;
      FT_Init_FreeType(&font.ftlib);
      FT_New_Face(font.ftlib, font_path, 0, &font.ftface);
      FT_Select_Charmap(font.ftface, FT_ENCODING_UNICODE);
      FT_Set_Pixel_Sizes(font.ftface, 0, font_size);
   }

//   font.monochrome = true;
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

      font.p.texture.width = font.atlas.slot_width << 4;
      font.p.texture.height = font.atlas.slot_height << 4;
      font.p.texture.format = VK_FORMAT_R8_UNORM;

      font.p.ssbo.info.range = sizeof(font_shader_storage_t);
      font.p.ubo.info.range = sizeof(font_uniforms_t);
      font.p.vbo.info.range = 4096 * sizeof(font_vertex_t);

      vk_pipeline_init(vk, &info, &font.p);
   }

   {
      device_memory_t *mem = &font.p.texture.staging.mem;
      memset(mem->u8 + mem->layout.offset, 0x0, mem->layout.size - mem->layout.offset);
   }

   font.p.texture.dirty = true;

   font_uniforms_t *uniforms = (font_uniforms_t *)font.p.ubo.mem.ptr;
   uniforms->tex_size.width = font.p.texture.width;
   uniforms->tex_size.height = font.p.texture.height;
   font.p.ubo.dirty = true;

   font.p.vbo.info.range = 0;
}

void vulkan_font_destroy(VkDevice device)
{
   FT_Done_Face(font.ftface);
   FT_Done_FreeType(font.ftlib);
   vk_pipeline_destroy(device, &font.p);
}


static int vulkan_font_get_new_slot(void)
{
   int i;
   unsigned oldest = 0;
   atlas_slot_t *const slot_map = font.atlas.slot_map;

   for (i = 1; i < 256; i++)
   {
      unsigned usage_counter = font.atlas.usage_counter;

      if ((usage_counter - slot_map[i].last_used) > (usage_counter - slot_map[oldest].last_used))
         oldest = i;
   }

   int map_id = font.atlas.slot_map[oldest].charcode & 0xFF;

   atlas_slot_t **const uc_map = font.atlas.uc_map;

   if (uc_map[map_id] == &slot_map[oldest])
      uc_map[map_id] = slot_map[oldest].next;
   else if (uc_map[map_id])
   {
      atlas_slot_t *ptr = uc_map[map_id];

      while (ptr->next && ptr->next != &slot_map[oldest])
         ptr = ptr->next;

      ptr->next = slot_map[oldest].next;
   }

   return oldest;
}

static void ft_font_render_glyph(unsigned charcode, int slot_id)
{
   int row;

   FT_Load_Char(font.ftface, charcode, FT_LOAD_RENDER | (font.monochrome ? FT_LOAD_MONOCHROME : 0));

   FT_Bitmap *bitmap = &font.ftface->glyph->bitmap;
   uint8_t *src = bitmap->buffer;
   device_memory_t *mem = &font.p.texture.staging.mem;
   uint8_t *dst = mem->u8 + mem->layout.offset + (slot_id & 0xF) * font.atlas.slot_width +
      (((slot_id >> 4) * font.atlas.slot_height)) * mem->layout.rowPitch;

   assert((dst - mem->u8 + mem->layout.rowPitch * (bitmap->rows + 1) < mem->layout.size));

   for (row = 0; row < bitmap->rows; row++)
   {
      if (font.monochrome)
      {
         int col;

         for (col = 0; col < bitmap->width; col++)
         {
            if (src[col >> 3] & (0x80 >> (col & 0x7)))
               dst[col] = 255;
         }
      }
      else
         memcpy(dst, src, bitmap->width);

      src += bitmap->pitch;
      dst += mem->layout.rowPitch;
   }

   font.p.texture.dirty = true;

   font_uniforms_t *uniforms = (font_uniforms_t *)font.p.ubo.mem.ptr;
   uniforms->glyph_metrics[slot_id].x = font.ftface->glyph->metrics.horiBearingX >> 6;
   uniforms->glyph_metrics[slot_id].y = -font.ftface->glyph->metrics.horiBearingY >> 6;
   uniforms->glyph_metrics[slot_id].width = font.ftface->glyph->metrics.width >> 6;
   uniforms->glyph_metrics[slot_id].height = font.ftface->glyph->metrics.height >> 6;
   uniforms->advance[slot_id] = font.ftface->glyph->metrics.horiAdvance >> 6;
   font.p.ubo.dirty = true;
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
            atlas_slot->last_used = ++font.atlas.usage_counter;
            return atlas_slot - font.atlas.slot_map;
         }

         atlas_slot = atlas_slot->next;
      }
   }

   int slot_id = vulkan_font_get_new_slot();
   font.atlas.slot_map[slot_id].charcode = charcode;
   font.atlas.slot_map[slot_id].next = font.atlas.uc_map[map_id];
   font.atlas.uc_map[map_id] = &font.atlas.slot_map[slot_id];

   ft_font_render_glyph(charcode, slot_id);

   font.atlas.slot_map[slot_id].last_used = ++font.atlas.usage_counter;
   return slot_id;
}

void vulkan_font_draw_text(const char *text, const font_render_options_t *options)
{
   const unsigned char *in = (const unsigned char *)text;
   const unsigned char *last_space = NULL;
   int last_space_vertex = 0;
   int last_space_x = 0;
   int pos = 0;

   if (options->lines)
      string_list_push(options->lines, text);

   font_vertex_t *out = NULL;

   if (!options->dry_run)
      out = (font_vertex_t *)(font.p.vbo.mem.u8 + font.p.vbo.info.range);

   font_vertex_t vertex;
   vertex.color = *(typeof(vertex.color)*)&options->color;
   vertex.position.x = options->x;
   vertex.position.y = options->y + font.ascender;

   while (*in && (!out || ((uint8_t *)out - font.p.vbo.mem.u8 < font.p.vbo.mem.size - sizeof(*out))))
   {
      if (vertex.position.y > options->max_height)
         out = NULL;

      if (!options->lines && !out)
         break;

      uint32_t charcode = *(in++);

      if (charcode == '\n')
      {
         vertex.position.x = 0;
         vertex.position.y += font.line_height;

         if (*in && options->lines)
            string_list_push(options->lines, (const char *)in);

         continue;
      }

      if (charcode == '\t')
      {
         vertex.position.x += 4 * font.max_advance;
         continue;
      }

      if (charcode == ' ')
      {
         last_space = in;
         last_space_vertex = pos;
         last_space_x = vertex.position.x;
      }

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

      if ((vertex.position.x + ((font_uniforms_t *)font.p.ubo.mem.ptr)->advance[slot_id]) > options->max_width)
//      if ((vertex.position.x + font.max_advance) > video.screen.width)
      {
         vertex.position.y += font.line_height;

         if (last_space && (last_space + 1 < in) &&
            (last_space + (options->max_width / (2 * font.max_advance))) > in)
         {
            vertex.position.x -= last_space_x;
            if (options->lines)
               string_list_push(options->lines, (const char *)last_space);

            if(out)
            {
               font_vertex_t *ptr = out + last_space_vertex + 1;
               while (ptr < out + pos)
               {
                  ptr->position.x -= last_space_x;
                  ptr->position.y = vertex.position.y;
                  ptr++;
               }
            }
            last_space = NULL;
         }
         else
         {
            if (*in && options->lines)
               string_list_push(options->lines, (const char *)in);

            vertex.position.x = 0;

            if (last_space == in)
               continue;
         }
      }
      if(out)
      {
         out[pos] = vertex;
         out[pos++].slot_id = slot_id;
      }
      vertex.position.x += ((font_uniforms_t *)font.p.ubo.mem.ptr)->advance[slot_id];
   }

   font.p.vbo.info.range += pos * sizeof(font_vertex_t);
}

void vulkan_font_update_assets(VkDevice device, VkCommandBuffer cmd)
{
   font_render_options_t options =
   {
      .max_width = video.screens[0].width,
      .max_height = video.screens[0].height,
   };

   char buffer[512];
   vulkan_font_draw_text(video.fps, &options);

   snprintf(buffer, sizeof(buffer), "[%c,%c,%c] %i, %i", input.pointer.touch1 ? '#' : ' ',
      input.pointer.touch2 ? '#' : ' ', input.pointer.touch3 ? '#' : ' ', input.pointer.x, input.pointer.y);
   options.y = 20;
   vulkan_font_draw_text(buffer, &options);

//   vulkan_font_draw_text(console_get(), 0, 100, video.screens[0].width);
//   static int text_pos_y = 100;

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
//      "ASCII line-feeds may be used to split a UTF-8 stream into lines; and ASCII NUL ", 0, text_pos_y, video.screens[0].width);

//   vulkan_font_draw_text("北海道の有名なかん光地、知床半島で、黒いキツネがさつえいされました。"
//      "地元斜里町の町立知床博物館が、タヌキをかんさつするためにおいていた自動さつえいカメラがき重なすがたをとらえました＝"
//      "写真・同館ていきょう。同館の学芸員も「はじめて見た」とおどろいています。"
//      "黒い毛皮のために昔、ゆ入したキツネの子そんとも言われてますが、はっきりしません。"
//      "北海道の先住みん族、アイヌのみん話にも黒いキツネが登場し、神せいな生き物とされているそうです。",
//      0, text_pos_y, video.screens[0].width);

//   vulkan_font_draw_text("gl_Position.xy = pos + 2.0 * vec2(0.0, glyph_metrics[c].w) / vp_size;", 0, 32, video.screens[0].width);

//   vulkan_font_draw_text("ettf", 40, 220, video.screens[0].width);

   if (font.p.texture.dirty)
      texture_update(device, cmd, &font.p.texture);

   if (font.p.vbo.dirty)
      buffer_flush(device, &font.p.vbo);

   if (font.p.ubo.dirty)
      buffer_flush(device, &font.p.ubo);

//   if (font.p.ssbo.dirty)
//      buffer_flush(device, &font.p.ssbo);
}

void vulkan_font_render(VkCommandBuffer cmd)
{
   if ((font.p.vbo.info.range - font.p.vbo.info.offset) == 0)
      return;

   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, font.p.handle);
   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, font.p.layout, 0, 1, &font.p.desc, 0, NULL);

   vkCmdBindVertexBuffers(cmd, 0, 1, &font.p.vbo.info.buffer, &font.p.vbo.info.offset);
   vkCmdDraw(cmd, (font.p.vbo.info.range - font.p.vbo.info.offset) / sizeof(font_vertex_t), 1, 0, 0);
   font.p.vbo.info.range = 0;

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
//   debug_log("(float*)atlas_ssbo.mem.ptr : %i\n", *(int*)atlas_ssbo.mem.ptr);
}
