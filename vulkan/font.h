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
} font_render_options_t;

typedef enum
{
   BLACK = 30,
   RED,
   GREEN,
   YELLOW,
   BLUE,
   MAGENTA,
   CYAN,
   LIGHT_GRAY,

   CONSOLE_COLOR_RESET = 39,

   DARK_GRAY = 90,
   LIGHT_RED,
   LIGHT_GREEN,
   LIGHT_YELLOW,
   LIGHT_BLUE,
   LIGHT_MAGENTA,
   LIGHT_CYAN,
   WHITE,
   CONSOLE_COLORS_MAX
} console_colors_t;

extern vk_renderer_t font_renderer;

void vk_font_draw_text(const char *text, const font_render_options_t *options);
