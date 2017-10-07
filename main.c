
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "common.h"
#include "interface.h"
#include "platform.h"
#include "video.h"


video_t video;

int main(int argc, char **argv)
{
   debug_log("main\n");

   platform_init();

   video = video_vulkan;
   video.init();

   module_info_t module = {};
   {
      module_init_info_t info =
      {
         .filename = "zz.gb"
      };

      module_init(&info, &module);
   }

   video.frame_set_size(module.output_width, module.output_height);

   int frames = 0;
   struct timespec start_time;
   clock_gettime(CLOCK_MONOTONIC, &start_time);

   while (platform.running)
   {
      module_run_info_t info = {};
      do
      {
         uint32_t dummy_audio[40000 + 2064];
         video.frame_get_buffer(&info.screen.ptr, &info.pitch);
         info.max_samples = 40000;
         info.sound_buffer.u32 = dummy_audio;
         module_run(&info);
//         debug_log("info.max_samples : %i", info.max_samples);
      }
      while(!info.frame_completed);

      video.frame();

      struct timespec end_time;
      clock_gettime(CLOCK_MONOTONIC, &end_time);
      float diff = (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0f;
      frames++;

      platform_handle_events();

      if (diff > 0.5f)
      {
         printf("\rfps: %f", frames / diff);
         frames = 0;
         start_time = end_time;
         fflush(stdout);
      }


   }

   printf("\n");



   module_destroy();
   video.destroy();
   platform_destroy();

   debug_log("main exit\n");
   return 0;
}
