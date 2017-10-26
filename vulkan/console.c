#include <stdio.h>

#include "common.h"
#include "input.h"
#include "slider.h"
#include "font.h"

void console_draw(void)
{
   static float pos = 1.0;
   static bool grab = false;
   static pointer_t old_pointer;
//   static int count = 0;
//   printf("more %i\n", count++);
   input.update();
   if(input.pointer.x < video.screens[1].width - 20&& !input.pointer.touch1 && old_pointer.touch1)
      printf("click\n");

   string_list_t* lines = string_list_create();
   font_render_options_t options =
   {
      .y = 100,
      .max_width = video.screens[1].width - 20,
      .max_height = video.screens[1].height,
      .lines = lines,
      .dry_run = true
   };
   vulkan_font_draw_text(console_get(), &options);
   options.lines = NULL;
   options.dry_run = false;

   int visible_lines = 33;
   float size;
   if(visible_lines < lines->count)
      size = (float)visible_lines / lines->count;
   else
      size = 1.0;

   if(input.pointer.touch1 && !old_pointer.touch1)
   {
      if ((input.pointer.x > video.screens[1].width - 20)
          && (input.pointer.x < video.screens[1].width))
      {
         if(!grab)
         {
            if (input.pointer.y < video.screens[1].height * (pos * (1.0 - size)))
            {
               pos = input.pointer.y / ((float)video.screens[1].height * (1.0 - size)) - (size / 2) / (1.0 - size);
               pos = pos > 0.0 ? pos < 1.0 ? pos : 1.0 : 0.0;
            }
            else if (input.pointer.y > video.screens[1].height * (pos * (1.0 - size) + size))
            {
               pos = input.pointer.y / ((float)video.screens[1].height * (1.0 - size)) - (size / 2) / (1.0 - size);
               pos = pos > 0.0 ? pos < 1.0 ? pos : 1.0 : 0.0;
            }
         }
         grab = true;
      }
   }
   else if(!input.pointer.touch1)
   {
      grab = false;
      pos = pos > 0.0 ? pos < 1.0 ? pos : 1.0 : 0.0;
   }

   if(grab)
   {
      pos += (input.pointer.y - old_pointer.y) / ((float)video.screens[1].height * (1.0 - size));
   }
   float real_pos = pos > 0.0 ? pos < 1.0 ? pos * (1.0 - size) : 1.0 - size : 0.0;


   char buffer[512];
   snprintf(buffer, sizeof(buffer), "\e[31m[%c] %i", grab ? '#' : ' ', input.pointer.y - old_pointer.y);
   options.y = 40;
   vulkan_font_draw_text(buffer, &options);

   old_pointer = input.pointer;

   options.y = 100;
   options.max_width = video.screens[0].width - 20;
   vulkan_font_draw_text(lines->data[(int)(0.5 + lines->count * real_pos)], &options);

   free(lines);
   vulkan_slider_add(video.screens[1].width - 20,0,20,video.screens[1].height,real_pos,size);

}
