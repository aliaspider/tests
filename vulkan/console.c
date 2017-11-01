#include <stdio.h>

#include "common.h"
#include "input.h"
#include "slider.h"
#include "font.h"

static int last_update_counter;
static void *console_cache;
static int console_cache_size;
static const char *last_line_pointer;
string_list_t *lines;
screen_t last_screen;
void console_draw(screen_t *screen)
{
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



   static float pos = 1.0;
   static bool grab = false;
   static pointer_t old_pointer;
//   static int count = 0;
//   printf("more %i\n", count++);
//   input.update();

   if (input.pointer.x < screen->width - 20 && !input.pointer.touch1 && old_pointer.touch1)
      printf("click\n");

   if (last_update_counter != console_update_counter || last_screen.width != screen->width)
   {
      free(lines);
      lines = string_list_create();
      {
         font_render_options_t options =
         {
            .y = 100,
            .max_width = screen->width - 20,
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
      float size;
      size = (float)visible_lines / lines->count;

      if (input.pointer.touch1 && !old_pointer.touch1)
      {
         if ((input.pointer.x > screen->width - 20)
               && (input.pointer.x < screen->width))
         {
            if (!grab)
            {
               if (input.pointer.y < screen->height * (pos * (1.0 - size)))
               {
                  pos = input.pointer.y / ((float)screen->height * (1.0 - size)) - (size / 2) / (1.0 - size);
                  pos = pos > 0.0 ? pos < 1.0 ? pos : 1.0 : 0.0;
               }
               else if (input.pointer.y > screen->height * (pos * (1.0 - size) + size))
               {
                  pos = input.pointer.y / ((float)screen->height * (1.0 - size)) - (size / 2) / (1.0 - size);
                  pos = pos > 0.0 ? pos < 1.0 ? pos : 1.0 : 0.0;
               }
            }

            grab = true;
         }
      }
      else if (!input.pointer.touch1)
      {
         grab = false;
         pos = pos > 0.0 ? pos < 1.0 ? pos : 1.0 : 0.0;
      }

      if (grab)
         pos += (input.pointer.y - old_pointer.y) / ((float)screen->height * (1.0 - size));

      float real_pos = pos > 0.0 ? pos < 1.0 ? pos * (1.0 - size) : 1.0 - size : 0.0;
      vk_slider_add(screen->width - 20, 0, 20, screen->height, real_pos, size);

      {
         char buffer[512];
         snprintf(buffer, sizeof(buffer), "\e[31m[%c] %i\n[%c]Vsync_button", grab ? '#' : ' ', input.pointer.y - old_pointer.y
                  , input.pad.meta.vsync ? '#' : ' ');

         font_render_options_t options =
         {
            .y = 40,
            .max_width = screen->width - 20,
            .max_height = screen->height,
         };
         vk_font_draw_text(buffer, &options);
      }
      {
         font_render_options_t options =
         {
            .y = 100,
            .max_width = screen->width - 20,
            .max_height = screen->height,
            .cache = &console_cache,
            .cache_size = console_cache_size,
         };
         const char *line = lines->data[(int)(0.5 + lines->count * real_pos)];

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
         .y = 100,
         .max_width = screen->width,
         .max_height = screen->height,
      };
      vk_font_draw_text(lines->data[0], &options);
   }

   old_pointer = input.pointer;

   last_update_counter = console_update_counter;
   last_screen = *screen;
}

