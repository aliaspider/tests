#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "d3d9_common.h"

#include "common.h"
#include "video.h"
#include "input.h"

static Direct3D9 d3d;
static Direct3DDevice9 device;
static Direct3DVertexBuffer9 vbo;
static Direct3DVertexShader9 vs;
static Direct3DPixelShader9 ps;

//static IDirect3D9Vtbl D3D9;
//static IDirect3DDevice9Vtbl D3DDev9;
//static ID3D10BlobVtbl D3DB;

static void video_init()
{
   DEBUG_LINE();
   D3DPRESENT_PARAMETERS d3dpp =
   {
      .Windowed = TRUE,
      .SwapEffect = D3DSWAPEFFECT_FLIP,
      .hDeviceWindow = video.screens[0].hwnd,
      .PresentationInterval = D3DPRESENT_INTERVAL_IMMEDIATE,
      .BackBufferFormat = D3DFMT_X8R8G8B8,
//      .BackBufferWidth = 1680,
//      .BackBufferHeight = 1050
   };
   d3d = Direct3DCreate9(D3D_SDK_VERSION);
   D3D9_CreateDevice(d3d, D3DADAPTER_DEFAULT, D3DDEVTYPE_HAL, video.screens[0].hwnd, D3DCREATE_SOFTWARE_VERTEXPROCESSING, &d3dpp, &device);

//   D3DDev9 = *device->lpVtbl;
   typedef struct vertex_t
   {
      float x,y,z;
      uint32_t color;
   }vertex_t;

   vertex_t vertices[] =
   {
       { 0.5f, -0.5f, 0.0f, D3DCOLOR_XRGB(0, 0, 255)},
       { 0.5f,  0.5f, 0.0f, D3DCOLOR_XRGB(0, 255, 0)},
       {-0.5f,  0.5f, 0.0f, D3DCOLOR_XRGB(255, 0, 0)},
   };


   D3D9_CreateVertexBuffer(device, sizeof(vertices), 0, D3DFVF_XYZ|D3DFVF_DIFFUSE, D3DPOOL_MANAGED, &vbo, NULL);
   void* vbo_ptr;
   D3D9_VertexBufferLock(vbo, 0, sizeof(vertices), &vbo_ptr, 0);
   memcpy(vbo_ptr, vertices, sizeof(vertices));
   D3D9_VertexBufferUnlock(vbo);

   D3D9_SetFVF(device, D3DFVF_XYZ|D3DFVF_DIFFUSE);
   D3D9_SetStreamSource(device, 0, vbo, 0, sizeof(vertex_t));

   D3DBlob vs_code;
   D3DBlob ps_code;
#ifdef DEBUG
   UINT compileflags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
   UINT compileflags = 0;
#endif
   {
      D3DBlob error_msg;
      CHECK_WINERR(D3DCompileFromFile(L"win/d3d9/shaders.hlsl", NULL, NULL, "VSMain", "vs_2_0", compileflags, 0, &vs_code, &error_msg));
      if(error_msg)
      {
         printf("D3DCompileFromFile failed :\n%s\n",(const char*)D3D_GetBufferPointer(error_msg));
         fflush(stdout);
         Release(error_msg);
      }
      CHECK_WINERR(D3DCompileFromFile(L"win/d3d9/shaders.hlsl", NULL, NULL, "PSMain", "ps_2_0", compileflags, 0, &ps_code, &error_msg));
      if(error_msg)
      {
         printf("D3DCompileFromFile failed :\n%s\n",(const char*)D3D_GetBufferPointer(error_msg));
         fflush(stdout);
         Release(error_msg);
      }
   }
   D3D9_CreateVertexShader(device, D3D_GetBufferPointer(vs_code), &vs);
   D3D9_CreatePixelShader(device, D3D_GetBufferPointer(ps_code), &ps);

//   D3D9_SetVertexShader(device, vs);
//   D3D9_SetPixelShader(device, ps);
}


static void video_render()
{
   D3DMATRIX identity = {._11 = 1.0, ._22 = 1.0, ._33 = 1.0, ._44 = 1.0};
//   DEBUG_LINE();
   D3D9_Clear(device, 0, NULL, D3DCLEAR_TARGET, D3DCOLOR_XRGB(0, 20, 200), 1.0f, 0);
   D3D9_BeginScene(device);
   D3D9_SetTransform(device, D3DTS_VIEW, &identity);
   D3D9_SetTransform(device, D3DTS_PROJECTION, &identity);
//   d3d9_
   D3D9_SetVertexShader(device, vs);
   D3D9_SetPixelShader(device, ps);
   D3D9_DrawPrimitive(device, D3DPT_TRIANGLELIST, 0, 3);
   D3D9_EndScene(device);
   D3D9_Present(device, NULL, NULL, NULL, NULL);



}

static void video_destroy()
{
   DEBUG_LINE();
   Release(vbo);
   Release(device);
   Release(d3d);
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

