
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

   u32 keys = hidKeysHeld();

   input.pad.meta.exit = !!(keys & KEY_TOUCH);
   input.pad.meta.vsync = !!(keys & KEY_ZR);
   input.pad.meta.filter = !!(keys & KEY_ZL);

   input.pad.buttons.A = !!(keys & KEY_A);
   input.pad.buttons.B = !!(keys & KEY_B);
   input.pad.buttons.X = !!(keys & KEY_X);
   input.pad.buttons.Y = !!(keys & KEY_Y);
   input.pad.buttons.start = !!(keys & KEY_START);
   input.pad.buttons.select = !!(keys & KEY_SELECT);
   input.pad.buttons.L = !!(keys & KEY_L);
   input.pad.buttons.R = !!(keys & KEY_R);
   input.pad.buttons.up = !!(keys & KEY_DUP);
   input.pad.buttons.down = !!(keys & KEY_DDOWN);
   input.pad.buttons.left = !!(keys & KEY_DLEFT);
   input.pad.buttons.right = !!(keys & KEY_DRIGHT);

}

const input_t input_3ds =
{
   .init = input_init,
   .destroy = input_destroy,
   .update = input_update
};
