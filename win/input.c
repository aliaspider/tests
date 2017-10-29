
#include <dinput.h>

#include "input.h"
#include "platform.h"
#include "video.h"
#include "common.h"

static LPDIRECTINPUT8 dinput;
static LPDIRECTINPUTDEVICE8 keyboard[MAX_SCREENS];
static LPDIRECTINPUTDEVICE8 mouse[MAX_SCREENS];

//char err_str[256];
//FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, HRESULT_CODE(hr), 0, err_str, sizeof(err_str), NULL);
//printf("%s\n", err_str);

#define CHECK_WINERR(x) do{HRESULT hr = x; if(FAILED(hr)) {\
   printf("error at %s:%i:%s: (0x%X, 0x%X, 0x%X) 0x%08X(%i)\n", \
   __FILE__, __LINE__, __FUNCTION__, HRESULT_SEVERITY(hr), HRESULT_FACILITY(hr), HRESULT_CODE(hr), hr, hr);\
   fflush(stdout);assert(0);}}while(0)

#define DEBUG_WINERR(x) do{printf("(0x%X, 0x%X, 0x%X) 0x%08X\n", HRESULT_SEVERITY(hr), HRESULT_FACILITY(hr), HRESULT_CODE(hr), hr);fflush(stdout);}while(0)

void input_init()
{
   CHECK_WINERR(DirectInput8Create(platform.hInstance, DIRECTINPUT_VERSION, &IID_IDirectInput8, (LPVOID*)&dinput, NULL));

   for (int i = 0; i < video.screen_count; i++)
   {
      CHECK_WINERR(IDirectInput8_CreateDevice(dinput, &GUID_SysKeyboard, &keyboard[i], NULL));
      CHECK_WINERR(IDirectInput8_CreateDevice(dinput, &GUID_SysMouse, &mouse[i], NULL));
      {
         DIDEVCAPS caps = {sizeof(caps)};
         CHECK_WINERR(IDirectInputDevice8_GetCapabilities(keyboard[i], &caps));
         CHECK_WINERR(IDirectInputDevice8_GetCapabilities(mouse[i], &caps));
      }

      CHECK_WINERR(IDirectInputDevice8_SetCooperativeLevel(keyboard[i], video.screens[i].hwnd,
                   DISCL_NONEXCLUSIVE | DISCL_FOREGROUND));

      CHECK_WINERR(IDirectInputDevice8_SetCooperativeLevel(mouse[i], video.screens[i].hwnd,
                   DISCL_NONEXCLUSIVE | DISCL_FOREGROUND));

      CHECK_WINERR(IDirectInputDevice8_SetDataFormat(keyboard[i], &c_dfDIKeyboard));
      CHECK_WINERR(IDirectInputDevice8_SetDataFormat(mouse[i], &c_dfDIMouse));

      IDirectInputDevice8_Acquire(keyboard[i]);
      IDirectInputDevice8_Acquire(mouse[i]);

   }


}

void input_destroy()
{
   for (int i = 0; i < video.screen_count; i++)
   {
      CHECK_WINERR(IDirectInputDevice8_Release(keyboard[i]));
      CHECK_WINERR(IDirectInputDevice8_Release(mouse[i]));
   }

   CHECK_WINERR(IDirectInput8_Release(dinput));

}

void input_update()
{
   int8_t keys[256];
   int i;

   for (i = 0; i < video.screen_count; i++)
   {
      if (FAILED(IDirectInputDevice8_GetDeviceState(keyboard[i], sizeof(keys), keys))
            && FAILED(IDirectInputDevice8_Acquire(keyboard[i])))
         continue;

      if (SUCCEEDED(IDirectInputDevice8_GetDeviceState(keyboard[i], sizeof(keys), keys)))
         break;
   }

   if (i == video.screen_count)
      return;

   input.pad.buttons.A = (keys[DIK_Q] >> 7);
   input.pad.buttons.B = (keys[DIK_W] >> 7);
   input.pad.buttons.X = (keys[DIK_A] >> 7);
   input.pad.buttons.Y = (keys[DIK_S] >> 7);
   input.pad.buttons.up = (keys[DIK_UP] >> 7);
   input.pad.buttons.down = (keys[DIK_DOWN] >> 7);
   input.pad.buttons.left = (keys[DIK_LEFT] >> 7);
   input.pad.buttons.right = (keys[DIK_RIGHT] >> 7);
   input.pad.buttons.L = (keys[DIK_E] >> 7);
   input.pad.buttons.R = (keys[DIK_D] >> 7);
   input.pad.buttons.start = (keys[DIK_RETURN] >> 7);
   input.pad.buttons.select = (keys[DIK_RSHIFT] >> 7);
   input.pad.meta.exit = (keys[DIK_ESCAPE] >> 7);
   input.pad.meta.menu = (keys[DIK_F4] >> 7);
   input.pad.meta.vsync = (keys[DIK_F5] >> 7);
   input.pad.meta.filter = (keys[DIK_F6] >> 7);

}
const input_t input_dinput =
{
   .init = input_init,
   .destroy = input_destroy,
   .update = input_update
};
