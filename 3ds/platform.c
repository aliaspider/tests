
#include <3ds.h>

#include "platform.h"
#include "common.h"
#include "video.h"

platform_t platform;
bool __ctru_speedup = true;
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
   wait_for_input();

   socInit(soc_buffer, sizeof(soc_buffer));
   httpcInit(0);
   amInit();
}

void platform_destroy()
{
   DEBUG_LINE();
   amExit();
   httpcExit();
   socExit();

   u8 param[0x300];
   u8 hmac[0x20];
   APT_PrepareToDoApplicationJump(0, 0x000400000BC00000ULL, MEDIATYPE_SD);
   APT_DoApplicationJump(param, sizeof(param), hmac);

   wait_for_input();
   gfxExit();
}

void platform_update()
{

}
