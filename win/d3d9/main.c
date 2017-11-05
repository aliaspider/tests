#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <d3d9.h>
#include "common.h"
#include "video.h"
#include "input.h"

typedef IDirect3D9* Direct3D9;
typedef IDirect3DDevice9* Direct3DDevice9;


Direct3D9 d3d;
Direct3DDevice9 device;

static void video_init()
{
   DEBUG_LINE();
   D3DPRESENT_PARAMETERS d3dpp =
   {
      .Windowed = TRUE,
      .SwapEffect = D3DSWAPEFFECT_FLIP,
//      .hDeviceWindow = video.screens[0].hwnd,
      .PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE,
   };
   d3d = Direct3DCreate9(D3D_SDK_VERSION);
   CHECK_WINERR(d3d->lpVtbl->CreateDevice(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, video.screens[0].hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device));



}


static void video_render()
{
//   DEBUG_LINE();
   device->lpVtbl->Clear(device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 20, 200), 1.0f, 0);
   device->lpVtbl->BeginScene(device);
   device->lpVtbl->EndScene(device);
   device->lpVtbl->Present(device, NULL, NULL, NULL, NULL);

}

static void video_destroy()
{
   DEBUG_LINE();
}

static void video_register_draw_command(int screen_id, draw_command_t fn)
{
}

const video_t video_d3d9 =
{
   .init                  = video_init,
   .render                = video_render,
   .destroy               = video_destroy,
   .register_draw_command = video_register_draw_command,
};

