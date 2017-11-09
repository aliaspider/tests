
#include <3ds.h>

#include "audio.h"
#include "common.h"
#include "interface.h"


static void audio_init()
{
   DEBUG_LINE();
}

static void audio_destroy()
{
   DEBUG_LINE();
}

static void audio_play(void *buffer, int samples)
{
}

const audio_t audio_3ds =
{
   .init = audio_init,
   .destroy = audio_destroy,
   .play = audio_play
};
