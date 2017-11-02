#include <stdio.h>

#include "common.h"
#include "input.h"
#include "slider.h"
#include "font.h"
#include "ui/slider.h"

static int last_update_counter;
static void *console_cache;
static int console_cache_size;
static const char *last_line_pointer;
string_list_t *lines;
screen_t last_screen;
void console_draw(screen_t *screen)
{
   const int slider_width = 20;
   {
      char buffer[512];
      snprintf(buffer, sizeof(buffer), "[%c,%c,%c] \e[91m%i, \e[32m%i", input.pointer.touch1 ? '#' : ' ',
               input.pointer.touch2 ? '#' : ' ', input.pointer.touch3 ? '#' : ' ', input.pointer.x, input.pointer.y);

      font_render_options_t options =
      {
         .y = 20,
         .max_width = screen->width,
         .max_height = screen->height,
      };
      vk_font_draw_text(buffer, &options);
//      vk_font_draw_text("北海道の有名なかん光地、知床半島で、黒いキツネがさつえいされました。"
//                        "地元斜里町の町立知床博物館が、タヌキをかんさつするためにおいていた自動さつえいカメラがき重なすがたをとらえました＝"
//                        "写真・同館ていきょう。同館の学芸員も「はじめて見た」とおどろいています。"
//                        "黒い毛皮のために昔、ゆ入したキツネの子そんとも言われてますが、はっきりしません。"
//                        "北海道の先住みん族、アイヌのみん話にも黒いキツネが登場し、神せいな生き物とされているそうです。",
//                        &options);
   }


   if (last_update_counter != console_update_counter || last_screen.width != screen->width)
   {
      free(lines);
      lines = string_list_create();
      {
         font_render_options_t options =
         {
            .y = 17 * 5,
            .max_width = screen->width - slider_width,
            .max_height = screen->height,
            .lines = lines,
            .dry_run = true
         };
         vk_font_draw_text(console_get(), &options);
      }
   }

   int visible_lines = (screen->height - 100) / 22;

   if (visible_lines < lines->count)
   {
      static slider_t slider;

      if (!slider.width)
      {
         slider.pos = 1.0;
         slider_init(&slider);
      }
      slider.width = slider_width;
      slider.height = screen->height;
      slider.x = screen->width - slider.width;
      slider.y = 0;
      slider.size = (float)visible_lines / lines->count;

      slider_update(&slider);

      {
         char buffer[512];
         snprintf(buffer, sizeof(buffer), "\e[31m[%c]\n[%c]Vsync_button", slider.grab ? '#' : ' ', input.pad.meta.vsync ? '#' : ' ');

         font_render_options_t options =
         {
            .y = 40,
            .max_width = screen->width - slider.width,
            .max_height = screen->height,
         };
         vk_font_draw_text(buffer, &options);
      }
      {
         font_render_options_t options =
         {
            .y = 17 * 5,
            .max_width = screen->width - slider.width,
            .max_height = screen->height,
            .cache = &console_cache,
            .cache_size = console_cache_size,
         };
         const char *line = lines->data[(int)(0.5 + lines->count * slider.real_pos)];

         if (last_update_counter != console_update_counter || line != last_line_pointer)
         {
            free(console_cache);
            console_cache = NULL;
         }

         last_line_pointer = line;

         vk_font_draw_text(line, &options);
         console_cache_size = options.cache_size;
      }
   }
   else
   {
      font_render_options_t options =
      {
         .y = 17 * 5,
         .max_width = screen->width,
         .max_height = screen->height,
      };
      vk_font_draw_text(lines->data[0], &options);
   }

   last_update_counter = console_update_counter;
   last_screen = *screen;
}

