
#include <stdio.h>
#include <stdbool.h>
#include <time.h>

#include "common.h"
#include "interface.h"
#include "platform.h"
#include "video.h"
#include "audio.h"
#include "input.h"
#include "vulkan/font.h"


video_t video;
audio_t audio;
input_t input;
module_info_t module;

int main(int argc, char** argv)
{
   debug_log("main\n");

   video = video_vulkan;
#ifdef __WIN32__
   audio = audio_win;
   input = input_dinput;
#elif defined(__linux__)
   audio = audio_alsa;
#ifdef HAVE_X11
   input = input_x11;
#endif
#endif

   video.screen_count = 2;
   video.screens[0].x = 600;
   video.screens[0].y = 400;
   video.screens[0].width = 640;
   video.screens[0].height = 480;
   video.screens[1].x = 0;
   video.screens[1].y = 0;
   video.screens[1].width = 800;
   video.screens[1].height = 500;
   video.screens[2].x = 400;
   video.screens[2].y = 800;
   video.screens[2].width = 256;
   video.screens[2].height = 224;
//   video.vsync = true;
   video.filter = true;

   for(int i = 0; i < video.screen_count; i++)
      video.screens[i].id = i;

   platform_init();

   {
      module_init_info_t info =
      {
         .filename = argv[1]
      };

      module_init(&info, &module);
   }

   video.init();
//   audio.init();
   input.init();

   int frames = 0;
   struct timespec start_time;
   clock_gettime(CLOCK_MONOTONIC, &start_time);

   while (true)
   {
      platform_update();
      {
         uint64_t old_mask = input.pad.mask;
         input.update();
         input.pad_pressed.mask = (input.pad.mask ^ old_mask) & input.pad.mask;
         input.pad_released.mask = (input.pad.mask ^ old_mask) & ~input.pad.mask;
      }

      if (input.pad_pressed.meta.vsync)
      {
         video.vsync = !video.vsync;
         display_message(60, 0, video.screens[0].height - 30, 1, "\e[%imV-Sync %s", YELLOW,  video.vsync ? "ON" : "OFF");
      }

      if (input.pad_pressed.meta.filter)
      {
         video.filter = !video.filter;
         display_message(60, 0, video.screens[0].height - 30, 1, "\e[%imFiltering %s", RED, video.filter ? "ON" : "OFF");
      }

      if (input.pad.meta.exit)
         break;

      if(input.pad_pressed.buttons.start)
         display_message(600, 160, 60, ~0, "start pressed at frame : %i", frames);

      module_run_info_t info = {};
      uint32_t sound_buffer[40000 + 2064];

      do
      {
         info.screen.ptr = video.frame.data;
         info.pitch = video.frame.pitch;
         info.max_samples = 40000;
         info.sound_buffer.u32 = sound_buffer;
         info.pad = &input.pad;
         module_run(&info);
         //         debug_log("info.max_samples : %i", info.max_samples);
      }
      while (!info.frame_completed);

//      audio.play(info.sound_buffer.ptr, info.max_samples);

      video.render();

      struct timespec end_time;
      clock_gettime(CLOCK_MONOTONIC, &end_time);
      float diff = (end_time.tv_sec - start_time.tv_sec) +
                   (end_time.tv_nsec - start_time.tv_nsec) / 1000000000.0f;
      frames++;

      snprintf(video.fps, sizeof(video.fps), "fps: %f", frames / diff);

      if (diff > 0.5f)
      {

//         snprintf(video.fps, sizeof(video.fps), "fps: %f", frames / diff);
////         debug_log("\r%s", video.fps);
         frames = 0;
         start_time = end_time;
//         fflush(stdout);
      }


   }

   debug_log("\n");

   module_destroy();
   input.destroy();
//   audio.destroy();
   video.destroy();
   platform_destroy();

   debug_log("main exit\n");

   return 0;
}

