
#include <3ds.h>

#include "input.h"
#include "video.h"
#include "common.h"
#include "interface.h"

static void input_init()
{
   DEBUG_LINE();
}

static void input_destroy()
{
   DEBUG_LINE();
}

static void input_update()
{
   hidScanInput();

   u32 kDown = hidKeysDown();

   if (kDown & KEY_B)
      input.pad.meta.exit = true;

}

const input_t input_3ds =
{
   .init = input_init,
   .destroy = input_destroy,
   .update = input_update
};
