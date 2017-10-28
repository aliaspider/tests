
#include "audio.h"

void audio_init (void)
{

}
void audio_destroy (void)
{

}
void audio_play(void *buffer, int samples)
{

}

const audio_t audio_win =
{
   .init = audio_init,
   .destroy = audio_destroy,
   .play = audio_play
};

