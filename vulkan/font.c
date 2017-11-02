
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
} shader_storage_t;

typedef struct
{
   vec2 tex_size;
   vec4 glyph_metrics[256];
   float advance[256];
} uniforms_t;

typedef struct
{
   u8 slot_id;
   struct
   {
      u8 r, g, b;
   } color;
   struct
   {
      float x, y;
   } position;
} vertex_t;

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
   bool monochrome;
   struct
   {
      uniforms_t *data;
      int slot_width;
      int slot_height;
      atlas_slot_t slot_map[256];
      atlas_slot_t *uc_map[256];
      unsigned usage_counter;
   } atlas;
} font;

static void vk_font_init(vk_context_t *vk)
{

   {
#ifdef WIN32
      const char *font_path = "C:/Windows/Fonts/consola.ttf";
#else
      const char *font_path = "/usr/share/fonts/TTF/DejaVuSansMono.ttf";
//      const char *font_path = "/usr/share/fonts/TTF/NotoSerif-Regular.ttf";
//      const char *font_path = "/usr/share/fonts/TTF/HanaMinA.ttf";
//      const char* font_path = "/usr/share/fonts/TTF/LiberationMono-Regular.ttf";
//      const char* font_path = "/usr/share/fonts/75dpi/courR18.pcf.gz";
//      const char* font_path = "/usr/share/fonts/WindowsFonts/cour.ttf";
#endif

      FT_UInt font_size = 18;
      CHECK_ERR(FT_Init_FreeType(&font.ftlib));
      CHECK_ERR(FT_New_Face(font.ftlib, font_path, 0, &font.ftface));
      CHECK_ERR(FT_Select_Charmap(font.ftface, FT_ENCODING_UNICODE));
      CHECK_ERR(FT_Set_Pixel_Sizes(font.ftface, 0, font_size));
   }

//   font.monochrome = true;
   font.line_height = font.ftface->size->metrics.height >> 6;
   font.ascender = font.ftface->size->metrics.ascender >> 6;
   font.max_advance = font.ftface->size->metrics.max_advance >> 6;
   font.atlas.slot_width = font.max_advance;
   font.atlas.slot_height = font.line_height;



   {

#define SHADER_FILE font
#include "shaders.h"

      static const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {.location = 0, .binding = 0, VK_FORMAT_R8_UINT, offsetof(vertex_t, slot_id)},
         {.location = 1, .binding = 0, VK_FORMAT_R8G8B8_UNORM, offsetof(vertex_t, color)},
         {.location = 2, .binding = 0, VK_FORMAT_R32G32_SFLOAT, offsetof(vertex_t, position)}
      };

      static const VkPipelineColorBlendAttachmentState blend_state =
      {
         .blendEnable = VK_TRUE,
         .srcColorBlendFactor = VK_SRC_ALPHA, VK_ONE_MINUS_SRC_ALPHA, VK_ADD,
         .srcAlphaBlendFactor = VK_SRC_ALPHA, VK_ONE_MINUS_SRC_ALPHA, VK_ADD,
         .colorWriteMask = VK_COLOR_COMPONENT_ALL
      };

      static const vk_renderer_init_info_t info =
      {
         SHADER_INFO,
         .attrib_count = countof(attrib_desc),
         .attrib_desc = attrib_desc,
         .topology = VK_PRIMITIVE_TOPOLOGY_POINT_LIST,
         .color_blend_attachement_state = &blend_state,
      };

      R_font.tex.width = font.atlas.slot_width << 4;
      R_font.tex.height = font.atlas.slot_height << 4;
      R_font.tex.format = VK_FORMAT_R8_UNORM;

      R_font.ssbo.info.range = sizeof(shader_storage_t);
      R_font.ubo.info.range = sizeof(uniforms_t);
      R_font.vbo.info.range = 4096 * sizeof(vertex_t);
      R_font.vertex_stride = sizeof(vertex_t);

      vk_renderer_init(vk, &info, &R_font);
   }

   {
      device_memory_t *mem = &R_font.tex.staging.mem;
      memset(mem->u8 + mem->layout.offset, 0x00, mem->layout.size - mem->layout.offset);
   }

   R_font.tex.dirty = true;

   font.atlas.data = (uniforms_t *)R_font.ubo.mem.ptr;
   font.atlas.data->tex_size.width = R_font.tex.width;
   font.atlas.data->tex_size.height = R_font.tex.height;
   R_font.ubo.dirty = true;
}

void vk_font_destroy(vk_renderer_t *renderer, VkDevice device)
{
   FT_Done_Face(font.ftface);
   FT_Done_FreeType(font.ftlib);
   memset(&font, 0, sizeof(font));

   vk_renderer_destroy(&R_font, device);
}


static inline int vulkan_font_get_new_slot(void)
{
   unsigned oldest = 0;
   atlas_slot_t *const slot_map = font.atlas.slot_map;

   for (int i = 1; i < 256; i++)
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

typedef struct
{
   u8 r, g, b;
} colorR8G8B8_t;


static colorR8G8B8_t console_colors[CONSOLE_COLORS_MAX] =
{
   [BLACK] =          {  0,   0,   0},
   [RED] =            {128,   0,   0},
   [GREEN] =          {  0, 128,   0},
   [YELLOW] =         {128, 128,   0},
   [BLUE] =           {  0,   0, 128},
   [MAGENTA] =        {128,   0, 128},
   [CYAN] =           {  0, 128, 128},
   [LIGHT_GRAY] =     {192, 192, 192},
   [DARK_GRAY] =      {128, 128, 128},
   [LIGHT_RED] =      {255,   0,   0},
   [LIGHT_GREEN] =    {  0, 255,   0},
   [LIGHT_YELLOW] =   {255, 255,   0},
   [LIGHT_BLUE] =     {  0,   0, 255},
   [LIGHT_MAGENTA] =  {255,   0, 255},
   [LIGHT_CYAN] =     {  0, 255, 255},
   [WHITE] =          {255, 255, 255},
};

static inline void ft_font_render_glyph(unsigned charcode, int slot_id)
{
   CHECK_ERR(FT_Load_Char(font.ftface, charcode, FT_LOAD_RENDER | (font.monochrome ? FT_LOAD_MONOCHROME : 0)));
   FT_Bitmap *bitmap = &font.ftface->glyph->bitmap;
   u8 *src = bitmap->buffer;
   device_memory_t *mem = &R_font.tex.staging.mem;
   u8 *dst = mem->u8 + mem->layout.offset + (slot_id & 0xF) * font.atlas.slot_width +
             (((slot_id >> 4) * font.atlas.slot_height)) * mem->layout.rowPitch;
   assert((dst - mem->u8 + mem->layout.rowPitch * (bitmap->rows + 1) < mem->layout.size));

   for (int row = 0; row < bitmap->rows; row++)
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

   R_font.tex.dirty = true;

   font.atlas.data->glyph_metrics[slot_id].x = font.ftface->glyph->metrics.horiBearingX >> 6;
   font.atlas.data->glyph_metrics[slot_id].y = -font.ftface->glyph->metrics.horiBearingY >> 6;
   font.atlas.data->glyph_metrics[slot_id].width = font.ftface->glyph->metrics.width >> 6;
   font.atlas.data->glyph_metrics[slot_id].height = font.ftface->glyph->metrics.height >> 6;
   font.atlas.data->advance[slot_id] = font.ftface->glyph->metrics.horiAdvance >> 6;
   R_font.ubo.dirty = true;
}

static inline int vulkan_font_get_slot_id(uint32_t charcode)
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

void vk_font_draw_text(const char *text, font_render_options_t *options)
{
   if (options->cache && *options->cache)
   {
      memcpy(vk_get_vbo_memory(&R_font.vbo, options->cache_size), *options->cache, options->cache_size);
//      fprintf(stdout, "cached draw\n"); fflush(stdout);
      return;
   }

   const unsigned char *in = (const unsigned char *)text;
   const unsigned char *last_space = NULL;
   int last_space_vertex = 0;
   int last_space_x = 0;
   int pos = 0;

   if (options->lines)
      options->lines = string_list_push(options->lines, text);

   vertex_t *out = NULL;

   if (!options->dry_run)
      out = (vertex_t *)(R_font.vbo.mem.u8 + R_font.vbo.info.offset + R_font.vbo.info.range);

   vertex_t vertex;
   vertex.color = *(typeof(vertex.color) *)&options->color;
   vertex.position.x = options->x;
   vertex.position.y = options->y + font.ascender;

   while (*in && (!out || ((u8 *)out - R_font.vbo.mem.u8 < R_font.vbo.mem.size - sizeof(*out))))
   {
      if (vertex.position.y > options->max_height)
         out = NULL;

      if (!options->lines && !out)
         break;

      uint32_t charcode = *(in++);

      if (charcode == 0x1B && *(in++) == '[')
      {
         int color_code = 0;

         while (*in != 'm')
            color_code = color_code * 10 + *(in++) - '0';

         if (color_code == CONSOLE_COLOR_RESET)
            vertex.color = *(typeof(vertex.color) *)&options->color;
         else if (color_code < CONSOLE_COLORS_MAX)
            vertex.color = *(typeof(vertex.color) *)&console_colors[color_code];

         in++;
         continue;
      }

      if (charcode == '\n')
      {
         vertex.position.x = 0;
         vertex.position.y += font.line_height;

         if (*in && options->lines)
            options->lines = string_list_push(options->lines, (const char *)in);

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

      if (vertex.position.x + font.atlas.data->advance[slot_id] > options->max_width)
//      if ((vertex.position.x + font.max_advance) > video.screen.width)
      {
         vertex.position.y += font.line_height;

         if (last_space && (last_space + 1 < in) &&
               (last_space + (options->max_width / (2 * font.max_advance))) > in)
         {
            vertex.position.x -= last_space_x;

            if (options->lines)
               options->lines = string_list_push(options->lines, (const char *)last_space);

            if (out)
            {
               vertex_t *ptr = out + last_space_vertex + 1;

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
               options->lines = string_list_push(options->lines, (const char *)in);

            vertex.position.x = 0;

            if (last_space == in)
               continue;
         }
      }

      if (out)
      {
         out[pos] = vertex;
         out[pos++].slot_id = slot_id;
      }

      vertex.position.x += font.atlas.data->advance[slot_id];
   }

   if (options->cache)
   {
      options->cache_size = pos * sizeof(vertex_t);
      *options->cache = malloc(options->cache_size);
      memcpy(*options->cache, R_font.vbo.mem.u8 + R_font.vbo.info.offset + R_font.vbo.info.range, options->cache_size);
   }

   R_font.vbo.info.range += pos * sizeof(vertex_t);
   R_font.vbo.dirty = true;
   assert(R_font.vbo.info.offset + R_font.vbo.info.range <= R_font.vbo.mem.size);
}

vk_renderer_t R_font =
{
   .init = vk_font_init,
   .destroy = vk_font_destroy,
   .begin = vk_renderer_begin,
   .finish = vk_renderer_finish,
};
