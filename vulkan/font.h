#pragma once

#include "vulkan_common.h"
#include "utils/string_list.h"

typedef struct
{
   int x;
   int y;
   int max_width;
   int max_height;
   struct
   {
      uint8_t r, g, b;
   } color;
   string_list_t* lines;
   bool dry_run;
   void** cache;
   int cache_size;
} font_render_options_t;

extern vk_renderer_t R_font;
extern vk_renderer_t R_monofont;

void vk_font_draw_text(const char *text, font_render_options_t *options);
void vk_monofont_draw_text(const char *text, int x, int y, uint32_t color, screen_t *screen);
