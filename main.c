
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <X11/Xutil.h>

#include "video.h"
#include "common.h"

int main(int argc, char **argv)
{
   debug_log("main\n");

   video_init();

   bool running = true;

   int frames = 0;
   struct timespec start_time;
   clock_gettime(CLOCK_MONOTONIC, &start_time);


   while (running)
   {
      video_frame();

      struct timespec end_time;
      clock_gettime(CLOCK_MONOTONIC, &end_time);
      float diff = (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0f;
      frames++;

      XEvent e;
      if(XCheckWindowEvent(video.display, video.window, ~0, &e) && (e.type == KeyPress))
      {
         if(e.xkey.keycode == XKeysymToKeycode(video.display, XK_q))
            running = false;
      }

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

   debug_log("main exit\n");
   return 0;
}
