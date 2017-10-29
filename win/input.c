
#include <dinput.h>

#include "input.h"
#include "platform.h"
#include "video.h"
#include "common.h"
#include "vulkan/font.h"

static LPDIRECTINPUT8 dinput;
static LPDIRECTINPUTDEVICE8 keyboards[MAX_SCREENS];
static LPDIRECTINPUTDEVICE8 mice[MAX_SCREENS];
static LPDIRECTINPUTDEVICE8 keyboard;
static LPDIRECTINPUTDEVICE8 mouse;
POINT origin_pos;
DIMOUSESTATE origin_state;
HWND active_window;

//char err_str[256];
//FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, HRESULT_CODE(hr), 0, err_str, sizeof(err_str), NULL);
//printf("%s\n", err_str);

#define CHECK_WINERR(x) do{HRESULT hr = x; if(FAILED(hr)) {\
   printf("error at %s:%i:%s: (0x%X, 0x%X, 0x%X) 0x%08X(%i)\n", \
   __FILE__, __LINE__, __FUNCTION__, HRESULT_SEVERITY(hr), HRESULT_FACILITY(hr), HRESULT_CODE(hr), hr, hr);\
   fflush(stdout);assert(0);}}while(0)

#define DEBUG_WINERR(x) do{printf("(0x%X, 0x%X, 0x%X) 0x%08X\n", HRESULT_SEVERITY(hr), HRESULT_FACILITY(hr), HRESULT_CODE(hr), hr);fflush(stdout);}while(0)

static void print_mouse_state(screen_t* screen)
{
   char mouse_state[512];
   snprintf(mouse_state, sizeof(mouse_state), "0x%08lX 0x%08lX 0x%08lX 0x%08lX", origin_pos.x, origin_pos.y,
            origin_state.lX, origin_state.lY);

   font_render_options_t options =
   {
      .x = 40,
      .y = 40,
      .max_width = screen->width,
      .max_height = screen->height,
   };
   vk_font_draw_text(mouse_state, &options);

}

void input_init()
{
   video.register_draw_command(1, print_mouse_state);

   CHECK_WINERR(DirectInput8Create(platform.hInstance, DIRECTINPUT_VERSION, &IID_IDirectInput8, (LPVOID*)&dinput, NULL));

   for (int i = 0; i < video.screen_count; i++)
   {
      CHECK_WINERR(IDirectInput8_CreateDevice(dinput, &GUID_SysKeyboard, &keyboards[i], NULL));
      CHECK_WINERR(IDirectInput8_CreateDevice(dinput, &GUID_SysMouse, &mice[i], NULL));
      {
         DIDEVCAPS caps = {sizeof(caps)};
         CHECK_WINERR(IDirectInputDevice8_GetCapabilities(keyboards[i], &caps));
         CHECK_WINERR(IDirectInputDevice8_GetCapabilities(mice[i], &caps));
      }

      CHECK_WINERR(IDirectInputDevice8_SetCooperativeLevel(keyboards[i], video.screens[i].hwnd,
                   DISCL_NONEXCLUSIVE | DISCL_FOREGROUND));

      CHECK_WINERR(IDirectInputDevice8_SetCooperativeLevel(mice[i], video.screens[i].hwnd,
                   DISCL_NONEXCLUSIVE | DISCL_FOREGROUND));

      CHECK_WINERR(IDirectInputDevice8_SetDataFormat(keyboards[i], &c_dfDIKeyboard));
#if 0
      CHECK_WINERR(IDirectInputDevice8_SetDataFormat(mice[i], &c_dfDIMouse));

      DIPROPDWORD props;
//      props.dwData
//      DIPROPAXISMODE_ABS
//      CHECK_WINERR(IDirectInputDevice8_SetProperty(mice[i], DIPROP_AXISMODE , ));
#else
      DIDATAFORMAT c_dfDIMouse_absaxis = c_dfDIMouse;
      c_dfDIMouse_absaxis.dwFlags = DIDF_ABSAXIS;
      CHECK_WINERR(IDirectInputDevice8_SetDataFormat(mice[i], &c_dfDIMouse_absaxis));
#endif

   }

   keyboard = keyboards[0];
   mouse = mice[0];


}

void input_destroy()
{
   for (int i = 0; i < video.screen_count; i++)
   {
      CHECK_WINERR(IDirectInputDevice8_Release(keyboards[i]));
      CHECK_WINERR(IDirectInputDevice8_Release(mice[i]));
   }

   CHECK_WINERR(IDirectInput8_Release(dinput));

}

void keyboard_update(void)
{
   int8_t state[256];

   if (FAILED(IDirectInputDevice8_GetDeviceState(keyboard, sizeof(state), state)))
   {
      for (int i = 0; i < video.screen_count; i++)
      {
         if (SUCCEEDED(IDirectInputDevice8_Acquire(keyboards[i])))
         {
            keyboard = keyboards[i];
            break;
         }
      }

      if (FAILED(IDirectInputDevice8_GetDeviceState(keyboard, sizeof(state), state)))
         return;
   }

   input.pad.buttons.A = (state[DIK_Q] >> 7);
   input.pad.buttons.B = (state[DIK_W] >> 7);
   input.pad.buttons.X = (state[DIK_A] >> 7);
   input.pad.buttons.Y = (state[DIK_S] >> 7);
   input.pad.buttons.up = (state[DIK_UP] >> 7);
   input.pad.buttons.down = (state[DIK_DOWN] >> 7);
   input.pad.buttons.left = (state[DIK_LEFT] >> 7);
   input.pad.buttons.right = (state[DIK_RIGHT] >> 7);
   input.pad.buttons.L = (state[DIK_E] >> 7);
   input.pad.buttons.R = (state[DIK_D] >> 7);
   input.pad.buttons.start = (state[DIK_RETURN] >> 7);
   input.pad.buttons.select = (state[DIK_RSHIFT] >> 7);
   input.pad.meta.exit = (state[DIK_ESCAPE] >> 7);
   input.pad.meta.menu = (state[DIK_F4] >> 7);
   input.pad.meta.vsync = (state[DIK_F5] >> 7);
   input.pad.meta.filter = (state[DIK_F6] >> 7);
}

void mouse_update(void)
{
   DIMOUSESTATE state;
   POINT pos;

   if (FAILED(IDirectInputDevice8_GetDeviceState(mouse, sizeof(state), &state)))
   {
      for (int i = 0; i < video.screen_count; i++)
      {
         if (SUCCEEDED(IDirectInputDevice8_Acquire(mice[i])))
         {
            if (FAILED(IDirectInputDevice8_GetDeviceState(mouse, sizeof(state), &state)))
            {
               IDirectInputDevice8_Release(mice[i]);
               return;
            }

            mouse = mice[i];
            active_window = video.screens[i].hwnd;
            GetCursorPos(&pos);
            ScreenToClient(active_window, &pos);
            origin_pos = pos;
            origin_state = state;
            break;
         }
      }
   }
   else
   {
      GetCursorPos(&pos);
      ScreenToClient(active_window, &pos);
   }

   input.pointer.x = pos.x;
   input.pointer.y = pos.y;
   input.pointer.touch1 = state.rgbButtons[0] & 0x80;
   input.pointer.touch2 = state.rgbButtons[1] & 0x80;

}


void input_update(void)
{
   keyboard_update();
   mouse_update();

}
const input_t input_dinput =
{
   .init = input_init,
   .destroy = input_destroy,
   .update = input_update
};
