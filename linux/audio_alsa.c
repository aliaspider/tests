

#include <string.h>
#include <fcntl.h>
#include <sys/ioctl.h>
#include <alsa/asoundlib.h>

#include "audio.h"
#include "common.h"
#include "interface.h"

static snd_pcm_t *pcm;

void alsa_init()
{
   //   u8 *sound_buffer;
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

}

void alsa_destroy()
{
   snd_pcm_drain(pcm);
   snd_pcm_close(pcm);
   pcm = NULL;
}

void alsa_play(void *buffer, int samples)
{

   {
      snd_pcm_sframes_t frames_written, frames;

      frames = snd_pcm_avail_update(pcm);

      if (frames < 0)
         frames = snd_pcm_recover(pcm, frames, 1);

      frames = samples;

      if (frames > 0)
      {
         frames_written = 0;

         //         while (frames_written < frames)
         {
            int result;

            int chunk_size = snd_pcm_avail_update(pcm);

            if (chunk_size > frames - frames_written)
               chunk_size = frames - frames_written;

            if (chunk_size)
            {
               result = snd_pcm_writei(pcm,
                                       (uint32_t *)buffer + frames_written,
                                       chunk_size);

               if (result < 0)
               {
                  debug_log("snd_pcm_writei error: %s\n", snd_strerror(result));
                  result = snd_pcm_recover(pcm, result, 1);

                  //                  if (result < 0)
                  //                     break;
               }
               else
                  frames_written += result;
            }
            else
               usleep(1000);
         }
      }
   }
}

const audio_t audio_alsa =
{
   .init = alsa_init,
   .destroy = alsa_destroy,
   .play = alsa_play
};
