
#include <stdio.h>
#include <stdbool.h>
#include <time.h>
#include <errno.h>
#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>

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
         //         .filename = "zz.gb"
//         .filename = "smw.sfc"
         .filename = test_file
      };

      module_init(&info, &module);
   }

   video.frame_init(module.output_width, module.output_height, module.screen_format);

   int frames = 0;
   struct timespec start_time;
   clock_gettime(CLOCK_MONOTONIC, &start_time);

   snd_pcm_t *pcm;
   //   uint8_t *sound_buffer;
   snd_pcm_sw_params_t *sw_params;
   snd_pcm_uframes_t alsa_buffer_size, alsa_period_size;

//   snd_lib_error_set_handler(snd_lib_error);
   CHECK_ERR(snd_pcm_open(&pcm, "default", SND_PCM_STREAM_PLAYBACK, SND_PCM_NONBLOCK));
   CHECK_ERR(snd_pcm_set_params(pcm, SND_PCM_FORMAT_S16, SND_PCM_ACCESS_RW_INTERLEAVED, 2,
                                module.audio_rate, 1, 40000));

   snd_pcm_sw_params_alloca(&sw_params);
   CHECK_ERR(snd_pcm_sw_params_current(pcm, sw_params));
   CHECK_ERR(snd_pcm_get_params(pcm, &alsa_buffer_size, &alsa_period_size));
   CHECK_ERR(snd_pcm_sw_params_set_start_threshold(pcm, sw_params,
                                         (alsa_buffer_size / alsa_period_size) * alsa_period_size));
   CHECK_ERR(snd_pcm_sw_params_set_avail_min(pcm, sw_params, alsa_period_size));
   CHECK_ERR(snd_pcm_sw_params(pcm, sw_params));

   while (platform.running)
   {
      module_run_info_t info = {};

      uint32_t sound_buffer[40000 + 2064];

      do
      {
         info.screen.ptr = video.frame.data;
         info.pitch = video.frame.pitch;
         info.max_samples = 40000;
         info.sound_buffer.u32 = sound_buffer;
         module_run(&info);
         //         debug_log("info.max_samples : %i", info.max_samples);
      }
      while (!info.frame_completed);

      {
         snd_pcm_sframes_t frames_written, frames;

         frames = snd_pcm_avail_update(pcm);

         if (frames < 0)
            frames = snd_pcm_recover(pcm, frames, 1);

         frames = info.max_samples;

         if (frames > 0)
         {
            frames_written = 0;

            while (frames_written < frames)
            {
               int result;

               int chunk_size = snd_pcm_avail_update(pcm);
               if(chunk_size > frames - frames_written)
                  chunk_size = frames - frames_written;

               if(chunk_size)
               {
                  result = snd_pcm_writei(pcm,
                                          sound_buffer + frames_written,
                                          chunk_size);

                  if (result < 0)
                  {
                     debug_log("snd_pcm_writei error: %s\n", snd_strerror(result));
                     result = snd_pcm_recover(pcm, result, 1);

                     if (result < 0)
                        break;
                  }
                  else
                     frames_written += result;
               }
               else
                  usleep(1000);
            }
         }
      }
      video.frame_update();

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
   snd_pcm_drain(pcm);
   snd_pcm_close(pcm);
   pcm = NULL;



   module_destroy();
   video.destroy();
   platform_destroy();

   debug_log("main exit\n");
   return 0;
}

