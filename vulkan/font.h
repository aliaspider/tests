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

void vulkan_font_init(vk_context_t* vk);
void vulkan_font_destroy(VkDevice device);
void vulkan_font_update_assets(VkDevice device, VkCommandBuffer cmd);
void vulkan_font_finish(VkDevice device);
void vulkan_font_render(VkCommandBuffer cmd);
void vulkan_font_draw_text(const char *text, const font_render_options_t *options);
