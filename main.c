
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "common.h"
#include "display.h"
#include "platform.h"
#include "video.h"

int main(int argc, char **argv)
{
   debug_log("main\n");

   platform_init();
   display_init(640,480);
   video_init();

   int frames = 0;
   struct timespec start_time;
   clock_gettime(CLOCK_MONOTONIC, &start_time);


   while (platform.running)
   {
      video_frame();

      struct timespec end_time;
      clock_gettime(CLOCK_MONOTONIC, &end_time);
      float diff = (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0f;
      frames++;

      platform_handle_events();

      if (diff > 0.5f)
      {
         printf("\r fps: %f", frames / diff);
         frames = 0;
         start_time = end_time;
         fflush(stdout);
      }


   }

   printf("\n");



   video_destroy();
   display_destroy();
   platform_destroy();

   debug_log("main exit\n");
   return 0;
}
