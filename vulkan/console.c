#include <stdio.h>

#include "common.h"
#include "input.h"
#include "slider.h"
#include "font.h"

void console_draw(screen_t* screen)
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
   }

//            static int text_pos_y = 100;

//            vulkan_font_draw_text("Backward compatibility: Backwards compatibility with ASCII and the enormous "
//               "amount of software designed to process ASCII-encoded text was the main driving "
//               "force behind the design of UTF-8. In UTF-8, single bytes with values in the range "
//               "of 0 to 127 map directly to Unicode code points in the ASCII range. Single bytes "
//               "in this range represent characters, as they do in ASCII.\n\nMoreover, 7-bit bytes "
//               "(bytes where the most significant bit is 0) never appear in a multi-byte sequence, "
//               "and no valid multi-byte sequence decodes to an ASCII code-point. A sequence of 7-bit "
//               "bytes is both valid ASCII and valid UTF-8, and under either interpretation represents "
//               "the same sequence of characters.\n\nTherefore, the 7-bit bytes in a UTF-8 stream represent "
//               "all and only the ASCII characters in the stream. Thus, many text processors, parsers, "
//               "protocols, file formats, text display programs etc., which use ASCII characters for "
//               "formatting and control purposes will continue to work as intended by treating the UTF-8 "
//               "byte stream as a sequence of single-byte characters, without decoding the multi-byte sequences. "
//               "ASCII characters on which the processing turns, such as punctuation, whitespace, and control "
//               "characters will never be encoded as multi-byte sequences. It is therefore safe for such "
//               "processors to simply ignore or pass-through the multi-byte sequences, without decoding them. "
//               "For example, ASCII whitespace may be used to tokenize a UTF-8 stream into words; "
//               "ASCII line-feeds may be used to split a UTF-8 stream into lines; and ASCII NUL ", 0, text_pos_y,
//               screen->width);

//            vulkan_font_draw_text("北海道の有名なかん光地、知床半島で、黒いキツネがさつえいされました。"
//               "地元斜里町の町立知床博物館が、タヌキをかんさつするためにおいていた自動さつえいカメラがき重なすがたをとらえました＝"
//               "写真・同館ていきょう。同館の学芸員も「はじめて見た」とおどろいています。"
//               "黒い毛皮のために昔、ゆ入したキツネの子そんとも言われてますが、はっきりしません。"
//               "北海道の先住みん族、アイヌのみん話にも黒いキツネが登場し、神せいな生き物とされているそうです。",
//               0, text_pos_y, screen->width);

//            vulkan_font_draw_text("gl_Position.xy = pos + 2.0 * vec2(0.0, glyph_metrics[c].w) / vp_size;", 0, 32,
//               screens[0].width);

//            vulkan_font_draw_text("ettf", 40, 220, screen->width);



   static float pos = 1.0;
   static bool grab = false;
   static pointer_t old_pointer;
//   static int count = 0;
//   printf("more %i\n", count++);
//   input.update();

   if (input.pointer.x < screen->width - 20 && !input.pointer.touch1 && old_pointer.touch1)
      printf("click\n");

   string_list_t *lines = string_list_create();
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
         };
         vk_font_draw_text(lines->data[(int)(0.5 + lines->count * real_pos)], &options);
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

   free(lines);
   old_pointer = input.pointer;

}
