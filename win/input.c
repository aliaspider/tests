
#include <dinput.h>

#include "input.h"
#include "platform.h"
#include "video.h"
#include "common.h"
#include "vulkan/font.h"

#define THIS__ dinput8
#define TYPE__ IDirectInput8*
#define PREFIX__ DI8_
IUNKNOWN__
WRAP(CreateDevice, REFGUID, guid, LPDIRECTINPUTDEVICE8*, DirectInputDevice, IUnknown*, UnkOuter)
WRAP(EnumDevices, u32, DevType, LPDIENUMDEVICESCALLBACK, Callback, void*, Ref, u32, Flags)
WRAP(GetDeviceStatus, REFGUID, rguidInstance)
WRAP(RunControlPanel, HWND, hwndOwner, u32, Flags)
WRAP(Initialize, HINSTANCE, hinst, u32, Version)
WRAP(FindDevice, REFGUID, guid, const TCHAR*, Name, LPGUID, pguidInstance)
WRAP(EnumDevicesBySemantics, const TCHAR*, UserName, LPDIACTIONFORMAT, ActionFormat, LPDIENUMDEVICESBYSEMANTICSCB,
     Callback, void*, Ref, u32, Flags)
WRAP(ConfigureDevices, LPDICONFIGUREDEVICESCALLBACK, diCallback, LPDICONFIGUREDEVICESPARAMS, diCDParams, u32, Flags,
     void*, RefData)
#undef THIS__
#undef TYPE__
#undef PREFIX__

#define THIS__ diDevice
#define TYPE__ IDirectInputDevice8*
#define PREFIX__ DIDEV8_
IUNKNOWN__
WRAP(GetCapabilities, LPDIDEVCAPS, DIDevCaps)
WRAP(EnumObjects, LPDIENUMDEVICEOBJECTSCALLBACK, Callback, void*, Ref, u32, Flags)
WRAP(GetProperty, REFGUID, rguidProp, LPDIPROPHEADER, pdiph)
WRAP(SetProperty, REFGUID, rguidProp, LPCDIPROPHEADER, pdiph)
WRAP(Acquire)
WRAP(Unacquire)
WRAP(GetDeviceState, u32, Size, void*, Data)
WRAP(GetDeviceData, u32, Size, DIDEVICEOBJECTDATA*, DevObjData, u32*, InOut, u32, Flags)
WRAP(SetDataFormat, LPCDIDATAFORMAT, df)
WRAP(SetEventNotification, HANDLE, hEvent)
WRAP(SetCooperativeLevel, HWND, hwnd, u32, Flags)
WRAP(GetObjectInfo, DIDEVICEOBJECTINSTANCE*, DevObjInstance, u32, Obj, u32, How)
WRAP(GetDeviceInfo, DIDEVICEINSTANCE*, DevInstance)
WRAP(RunControlPanel, HWND, hwndOwner, u32, Flags)
WRAP(Initialize, HINSTANCE, hinst, u32, Version, REFGUID, guid)
WRAP(CreateEffect, REFGUID, guid, LPCDIEFFECT, eff, LPDIRECTINPUTEFFECT*, ppdeff, IUnknown*, punkOuter)
WRAP(EnumEffects, LPDIENUMEFFECTSCALLBACKA, Callback, void*, Ref, u32, EffType)
WRAP(GetEffectInfo, DIEFFECTINFO*, EffectInfo, REFGUID, guid)
WRAP(GetForceFeedbackState, u32*, Out)
WRAP(SendForceFeedbackCommand, u32, Flags)
WRAP(EnumCreatedEffectObjects, LPDIENUMCREATEDEFFECTOBJECTSCALLBACK, Callback, void*, Ref, u32, fl)
WRAP(Escape, LPDIEFFESCAPE, pesc)
WRAP(Poll)
WRAP(SendDeviceData, u32, Size, const DIDEVICEOBJECTDATA*, DevObjData, u32*, pInOut, u32, fl)
WRAP(EnumEffectsInFile, const TCHAR*, FileName, LPDIENUMEFFECTSINFILECALLBACK, pec, void*, Ref, u32, Flags)
WRAP(WriteEffectToFile, const TCHAR*, FileName, u32, Entries, DIFILEEFFECT*, diFileEft, u32, Flags)
WRAP(BuildActionMap, DIACTIONFORMAT*, ActionFormat, const TCHAR*, UserName, u32, Flags)
WRAP(SetActionMap, DIACTIONFORMAT*, ActionFormat, const TCHAR*, UserName, u32, Flags)
WRAP(GetImageInfo, DIDEVICEIMAGEINFOHEADER*, diDevImageInfoHeader)
#undef THIS__
#undef TYPE__
#undef PREFIX__

static IDirectInput8* dinput;
static IDirectInputDevice8* keyboards[MAX_SCREENS];
static IDirectInputDevice8* mice[MAX_SCREENS];
static IDirectInputDevice8* keyboard;
static IDirectInputDevice8* mouse;

POINT origin_pos;
DIMOUSESTATE origin_state;
HWND active_window;

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

   CHECK_WINERR(DirectInput8Create(platform.hInstance, DIRECTINPUT_VERSION, &IID_IDirectInput8, (void**)&dinput, NULL));

   for (int i = 0; i < video.screen_count; i++)
   {
      CHECK_WINERR(DI8_CreateDevice(dinput, &GUID_SysKeyboard, &keyboards[i], NULL));
      CHECK_WINERR(DI8_CreateDevice(dinput, &GUID_SysMouse, &mice[i], NULL));
      {
         DIDEVCAPS caps = {sizeof(caps)};
         CHECK_WINERR(DIDEV8_GetCapabilities(keyboards[i], &caps));
         CHECK_WINERR(DIDEV8_GetCapabilities(mice[i], &caps));
      }

      CHECK_WINERR(DIDEV8_SetCooperativeLevel(keyboards[i], video.screens[i].hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND));

      CHECK_WINERR(DIDEV8_SetCooperativeLevel(mice[i], video.screens[i].hwnd, DISCL_NONEXCLUSIVE | DISCL_FOREGROUND));

      CHECK_WINERR(DIDEV8_SetDataFormat(keyboards[i], &c_dfDIKeyboard));
#if 0
      CHECK_WINERR(DIDEV8_SetDataFormat(mice[i], &c_dfDIMouse));

      DIPROPDWORD props;
//      props.dwData
//      DIPROPAXISMODE_ABS
//      CHECK_WINERR(DIDEV8_SetProperty(mice[i], DIPROP_AXISMODE , ));
#else
      DIDATAFORMAT c_dfDIMouse_absaxis = c_dfDIMouse;
      c_dfDIMouse_absaxis.dwFlags = DIDF_ABSAXIS;
      CHECK_WINERR(DIDEV8_SetDataFormat(mice[i], &c_dfDIMouse_absaxis));
#endif

   }

   keyboard = keyboards[0];
   mouse = mice[0];


}

void input_destroy()
{
   for (int i = 0; i < video.screen_count; i++)
   {
      CHECK_WINERR(DIDEV8_Release(keyboards[i]));
      CHECK_WINERR(DIDEV8_Release(mice[i]));
   }

   CHECK_WINERR(DI8_Release(dinput));

}

void keyboard_update(void)
{
   int8_t state[256];

   if (FAILED(DIDEV8_GetDeviceState(keyboard, sizeof(state), state)))
   {
      for (int i = 0; i < video.screen_count; i++)
      {
         if (SUCCEEDED(DIDEV8_Acquire(keyboards[i])))
         {
            keyboard = keyboards[i];
            break;
         }
      }

      if (FAILED(DIDEV8_GetDeviceState(keyboard, sizeof(state), state)))
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

   if (FAILED(DIDEV8_GetDeviceState(mouse, sizeof(state), &state)))
   {
      for (int i = 0; i < video.screen_count; i++)
      {
         if (SUCCEEDED(DIDEV8_Acquire(mice[i])))
         {
            if (FAILED(DIDEV8_GetDeviceState(mouse, sizeof(state), &state)))
            {
//               DIDEV8_Release(mice[i]);
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
