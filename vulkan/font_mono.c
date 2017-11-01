
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
#include "slider.h"

#define VK_ATLAS_WIDTH  512
#define VK_ATLAS_HEIGHT 512


typedef struct
{
   vec2 glyph_size;
   vec2 tex_size;
} uniforms_t;

typedef struct
{
   u8 slot_id;
   uint32_t color;
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
   int glyph_width;
   int glyph_height;
   int ascender;
   bool monochrome;
   struct
   {
      atlas_slot_t slot_map[256];
      atlas_slot_t *uc_map[256];
      unsigned usage_counter;
   } atlas;
} font;

static struct
{
   int width;
   int height;
   int rows;
   int cols;
   int count;
   int offset;
   int range;
   vertex_t *vbo;
} screen_data[MAX_SCREENS];

static void vk_monofont_init(vk_context_t *vk)
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
   font.glyph_width = font.ftface->size->metrics.max_advance >> 6;
   font.glyph_height = font.ftface->size->metrics.height >> 6;
   font.ascender = font.ftface->size->metrics.ascender >> 6;

   {

#define SHADER_FILE font_mono
#include "shaders.h"

      static const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {.location = 0, .binding = 0, VK_FORMAT_R8_UINT, offsetof(vertex_t, slot_id)},
         {.location = 1, .binding = 0, VK_FORMAT_R8G8B8A8_UNORM, offsetof(vertex_t, color)},
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

      R_monofont.tex.width = font.glyph_width << 4;
      R_monofont.tex.height = font.glyph_height << 4;
      R_monofont.tex.format = VK_FORMAT_R8_UNORM;

      R_monofont.ubo.info.range = sizeof(uniforms_t);
      R_monofont.vbo.info.range = video.screen_count * 200 * 100 * sizeof(vertex_t);
      R_monofont.vertex_stride = sizeof(vertex_t);

      vk_renderer_init(vk, &info, &R_monofont);
   }

   {
      device_memory_t *mem = &R_monofont.tex.staging.mem;
      memset(mem->u8 + mem->layout.offset, 0x00, mem->layout.size - mem->layout.offset);
   }
   {
      device_memory_t *mem = &R_monofont.vbo.mem;
      memset(mem->u8 + mem->layout.offset, 0x00, mem->layout.size - mem->layout.offset);
      R_monofont.vbo.dirty = true;
   }

//   R_monofont.tex.dirty = true;

   uniforms_t *uniforms = (uniforms_t *)R_monofont.ubo.mem.ptr;
   uniforms->tex_size.width = R_monofont.tex.width;
   uniforms->tex_size.height = R_monofont.tex.height;
   uniforms->glyph_size.width = font.glyph_width;
   uniforms->glyph_size.height = font.glyph_height;
   R_monofont.ubo.dirty = true;
}

void vk_monofont_destroy(VkDevice device, vk_renderer_t *this)
{
   FT_Done_Face(font.ftface);
   FT_Done_FreeType(font.ftlib);
   memset(&font, 0, sizeof(font));
   memset(&screen_data, 0, sizeof(screen_data));

   vk_renderer_destroy(device, &R_monofont);
}


static inline int vulkan_monofont_get_new_slot(void)
{
   unsigned oldest = 1;
   atlas_slot_t *const slot_map = font.atlas.slot_map;

   for (int i = 2; i < 256; i++)
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

static uint32_t console_colors[CONSOLE_COLORS_MAX] =
{
   [BLACK] =          0xFF000000,
   [RED] =            0xFF000080,
   [GREEN] =          0xFF008000,
   [YELLOW] =         0xFF008080,
   [BLUE] =           0xFF800080,
   [MAGENTA] =        0xFF800080,
   [CYAN] =           0xFF808000,
   [LIGHT_GRAY] =     0xFFC0C0C0,
   [DARK_GRAY] =      0xFF808080,
   [LIGHT_RED] =      0xFF0000FF,
   [LIGHT_GREEN] =    0xFF00FF00,
   [LIGHT_YELLOW] =   0xFFFFFF00,
   [LIGHT_BLUE] =     0xFFFF0000,
   [LIGHT_MAGENTA] =  0xFFFF00FF,
   [LIGHT_CYAN] =     0xFFFFFF00,
   [WHITE] =          0xFFFFFFFF,
};

static inline void ft_monofont_render_glyph(unsigned charcode, int slot_id)
{
   CHECK_ERR(FT_Load_Char(font.ftface, charcode, FT_LOAD_RENDER | (font.monochrome ? FT_LOAD_MONOCHROME : 0)));
   FT_Bitmap *bitmap = &font.ftface->glyph->bitmap;
   u8 *src = bitmap->buffer;
   device_memory_t *mem = &R_monofont.tex.staging.mem;
   int x = (slot_id & 0xF) * font.glyph_width + (font.ftface->glyph->metrics.horiBearingX >> 6);
   int y = (slot_id >> 4) * font.glyph_height + font.ascender - (font.ftface->glyph->metrics.horiBearingY >> 6);
   u8 *dst = mem->u8 + mem->layout.offset + x + y * mem->layout.rowPitch;
   assert((dst - mem->u8 + mem->layout.rowPitch * bitmap->rows  < mem->layout.size));

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

   R_monofont.tex.dirty = true;
}

static inline int vulkan_monofont_get_slot_id(uint32_t charcode)
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

   int slot_id = vulkan_monofont_get_new_slot();
   font.atlas.slot_map[slot_id].charcode = charcode;
   font.atlas.slot_map[slot_id].next = font.atlas.uc_map[map_id];
   font.atlas.uc_map[map_id] = &font.atlas.slot_map[slot_id];

   ft_monofont_render_glyph(charcode, slot_id);

   font.atlas.slot_map[slot_id].last_used = ++font.atlas.usage_counter;
   return slot_id;
}

static inline void update_screen_data(screen_t *screen)
{
   if (screen->width == screen_data[screen->id].width && screen->height == screen_data[screen->id].height)
      return;

   vertex_t *vbo = (vertex_t *)R_monofont.vbo.mem.ptr;

   for (int i = 0; i < video.screen_count; i++)
   {
      if (i >= screen->id)
      {
         screen_data[i].width = screen->width;
         screen_data[i].height = screen->height;
         screen_data[i].cols = screen->width / font.glyph_width;
         screen_data[i].rows = screen->height / font.glyph_height;
         screen_data[i].count = screen_data[i].rows * screen_data[i].cols;
         screen_data[i].vbo = vbo;
         screen_data[i].offset = (uint8_t *)vbo - R_monofont.vbo.mem.u8;
         vbo += screen_data[i].count;
         screen_data[i].range = (uint8_t *)vbo - R_monofont.vbo.mem.u8;

         memset(screen_data[i].vbo, 0x00, screen_data[i].count * sizeof(vertex_t));
      }
      else
         vbo += screen_data[i].count;
   }
}

static inline string_list_t *vk_monofont_get_lines(const char *text, int line_width)
{
   string_list_t *lines = string_list_create();
   int col = 0;
   int row = 0;
   lines = string_list_push(lines, text);

   while (*text)
   {
      const char c = *(text++);

      if (c == '\e')
      {
         while (*(text++) != 'm');

         continue;
      }

      if (c & 0x80)
      {
         while ((*(text) & 0xC0) == 0x80)
            text++;

      }

      if (c == '\t')
      {
         col += 3;
         continue;
      }

      col++;

      if (col >= line_width || c == '\n')
      {
         col = 0;
         row++;
         lines = string_list_push(lines, text);
      }
      if(c == '\n')
         fflush(stdout);
   }
   return lines;
}
void vk_monofont_draw_text(const char *text, int x, int y, uint32_t color, screen_t *screen)
{
   update_screen_data(screen);
   R_monofont.vbo.info.offset = screen_data[screen->id].offset;
   R_monofont.vbo.info.range = screen_data[screen->id].range;
   assert(R_monofont.vbo.info.range <= R_monofont.vbo.mem.size);

   vertex_t *out = screen_data[screen->id].vbo;
   out += x + y * screen_data[screen->id].cols;
   uint32_t current_color = color;

   const unsigned char *in = (const unsigned char *)text;

   while (*in && out - screen_data[screen->id].vbo < screen_data[screen->id].count)
   {
      uint32_t charcode = *(in++);

      if (charcode == '\e' && *(in++) == '[')
      {
         int color_code = 0;

         while (*in != 'm') /* check for 0 ? */
            color_code = color_code * 10 + *(in++) - '0';

         if (color_code == CONSOLE_COLOR_RESET)
            current_color = color;
         else if (color_code < CONSOLE_COLORS_MAX)
            current_color = console_colors[color_code];

         in++;
         continue;
      }

      if (charcode == '\n')
      {
         vertex_t *next_line = screen_data[screen->id].vbo + screen_data[screen->id].cols *
                               (1 + ((out - screen_data[screen->id].vbo) / screen_data[screen->id].cols));

         while (out < next_line)
            (out++)->slot_id = 0;

         continue;
      }

      if (charcode == '\t')
      {
         (out++)->slot_id = 0;
         (out++)->slot_id = 0;
         (out++)->slot_id = 0;
         (out++)->slot_id = 0;
         continue;
      }

      if (charcode == ' ')
      {
         out->slot_id = 0;
         out ++;
         continue;
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

      int slot_id = vulkan_monofont_get_slot_id(charcode);

      out->slot_id = slot_id;
      out->color = current_color;
      out++;
   }

   assert(out - 1 - screen_data[screen->id].vbo < screen_data[screen->id].count);


   R_monofont.vbo.dirty = true;
}

void console_mono_draw(screen_t *screen)
{

   static float pos[MAX_SCREENS] = {1.0, 1.0, 1.0, 1.0};
   static bool grab[MAX_SCREENS];
   static pointer_t old_pointer[MAX_SCREENS];

   if (input.pointer.x < screen->width - 20 && !input.pointer.touch1 && old_pointer[screen->id].touch1)
      printf("click\n");

   const char* text = console_get();
   update_screen_data(screen);

   string_list_t *lines = vk_monofont_get_lines(text, screen_data[screen->id].cols);
//   string_list_t *lines = string_list_create();
//   lines = string_list_push(lines, text);
   int visible_lines = screen_data[screen->id].rows - 4;

   if (visible_lines < lines->count)
   {
      float size;
      size = (float)visible_lines / lines->count;

      if (input.pointer.touch1 && !old_pointer[screen->id].touch1)
      {
         if ((input.pointer.x > screen->width - 20)
               && (input.pointer.x < screen->width))
         {
            if (!grab[screen->id])
            {
               if (input.pointer.y < screen->height * (pos[screen->id] * (1.0 - size)))
               {
                  pos[screen->id] = input.pointer.y / ((float)screen->height * (1.0 - size)) - (size / 2) / (1.0 - size);
                  pos[screen->id] = pos[screen->id] > 0.0 ? pos[screen->id] < 1.0 ? pos[screen->id] : 1.0 : 0.0;
               }
               else if (input.pointer.y > screen->height * (pos[screen->id] * (1.0 - size) + size))
               {
                  pos[screen->id] = input.pointer.y / ((float)screen->height * (1.0 - size)) - (size / 2) / (1.0 - size);
                  pos[screen->id] = pos[screen->id] > 0.0 ? pos[screen->id] < 1.0 ? pos[screen->id] : 1.0 : 0.0;
               }
            }

            grab[screen->id] = true;
         }
      }
      else if (!input.pointer.touch1)
      {
         grab[screen->id] = false;
         pos[screen->id] = pos[screen->id] > 0.0 ? pos[screen->id] < 1.0 ? pos[screen->id] : 1.0 : 0.0;
      }

      if (grab[screen->id])
         pos[screen->id] += (input.pointer.y - old_pointer[screen->id].y) / ((float)screen->height * (1.0 - size));

      float real_pos = pos[screen->id] > 0.0 ? pos[screen->id] < 1.0 ? pos[screen->id] * (1.0 - size) : 1.0 - size : 0.0;
      vk_slider_add(screen->width - 20, 0, 20, screen->height, real_pos, size);

      text = lines->data[(int)(0.5 + lines->count * real_pos)];
   }

   static int last_update_counter;
   static const char *last_line_pointer[MAX_SCREENS];

   if (last_update_counter != console_update_counter || last_line_pointer[screen->id] != text)
   {
      last_update_counter = console_update_counter;
      last_line_pointer[screen->id] = text;
      fprintf(stdout, "no skip\n");
      fflush(stdout);
   }
   else
      text = "";

   vk_monofont_draw_text(text, 0, 4, 0xFFFFFFFF, screen);

   old_pointer[screen->id] = input.pointer;


   free(lines);


}


vk_renderer_t R_monofont =
{
   .init = vk_monofont_init,
   .destroy = vk_monofont_destroy,
   .exec = vk_renderer_exec_simple,
   .flush = vk_renderer_flush,
};
