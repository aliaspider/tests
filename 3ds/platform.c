
#include <3ds.h>

#include "platform.h"
#include "common.h"
#include "video.h"
#include "input.h"

platform_t platform;
bool __ctru_speedup = true;
u32 __stacksize__ = 0x100000;
void wait_for_input(void)
{
   printf("\n\nPress Start.\n\n");
   fflush(stdout);

//   osSetSpeedupEnable(true);
   while (aptMainLoop())
   {
      hidScanInput();

      u32 kDown = hidKeysDown();

      if (kDown & KEY_START)
         break;

      if (kDown & KEY_SELECT)
         exit(0);

      svcSleepThread(1000000);
   }
}

__attribute((aligned(0x1000)))
static u32 soc_buffer[0x100000 >> 2];

void platform_init()
{
   DEBUG_LINE();
   gfxInit(GSP_BGR8_OES, GSP_RGB565_OES, false);

   gfxSet3D(false);
   consoleInit(GFX_BOTTOM, NULL);
#ifdef DEBUG
   wait_for_input();
#endif

   socInit(soc_buffer, sizeof(soc_buffer));
}

static u64 chainload_tid = 0x000400000BC00000ULL;
void platform_destroy()
{
   DEBUG_LINE();
   socExit();
   gfxExit();

   if(chainload_tid)
   {
      u8 param[0x300];
      u8 hmac[0x20];
      APT_PrepareToDoApplicationJump(0, chainload_tid, MEDIATYPE_SD);
      APT_DoApplicationJump(param, sizeof(param), hmac);

      svcSleepThread(1000000000);
   }
}

void platform_update()
{
   if(!aptMainLoop())
   {
      chainload_tid = 0;
      input.pad.meta.exit = true;
   }
}
