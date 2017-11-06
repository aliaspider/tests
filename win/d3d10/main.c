#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <d3d10.h>

#include "common.h"
#include "video.h"
#include "input.h"

static ID3D10Device *device;
static IDXGISwapChain *swapchain;
static ID3D10RenderTargetView *rt_view;
static ID3D10DeviceVtbl D3D10;
static IDXGISwapChainVtbl DXGI;
static void video_init()
{
   DEBUG_LINE();
   {
      DXGI_SWAP_CHAIN_DESC desc =
      {
         .BufferCount = 1,
         .BufferDesc.Width = video.screens[0].width,
         .BufferDesc.Height = video.screens[0].height,
         .BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
         .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
         .OutputWindow = video.screens[0].hwnd,
         .SampleDesc.Count = 1,
         .SampleDesc.Quality = 0,
         .Windowed = TRUE,
      };
      CHECK_WINERR(D3D10CreateDeviceAndSwapChain(NULL, D3D10_DRIVER_TYPE_HARDWARE, NULL, 0,
                   D3D10_SDK_VERSION, &desc,
                   &swapchain, &device));
   }
   D3D10 = *device->lpVtbl;
   DXGI = *swapchain->lpVtbl;

   ID3D10Texture2D *backbuffer;
   CHECK_WINERR(DXGI.GetBuffer(swapchain, 0, __uuidof(ID3D10Texture2D), (void **)&backbuffer));
   CHECK_WINERR(D3D10.CreateRenderTargetView(device, (ID3D10Resource*)backbuffer, NULL, &rt_view));
   Release(backbuffer);
   D3D10.OMSetRenderTargets(device, 1, &rt_view, NULL);

   D3D10_VIEWPORT vp = {0, 0, video.screens[0].width, video.screens[0].height, 0.0f, 1.0f};
   D3D10.RSSetViewports(device, 1, &vp);




}


static void video_render()
{
   float clearcolor[4] = {0.5, 0.2, 1.0, 1.0};
   D3D10.ClearRenderTargetView(device, rt_view, clearcolor);
   DXGI.Present(swapchain, 0, 0);

}

static void video_destroy()
{
   DEBUG_LINE();
}

static void video_register_draw_command(int screen_id, draw_command_t fn)
{
}


const video_t video_d3d10 =
{
   .init                  = video_init,
   .render                = video_render,
   .destroy               = video_destroy,
   .register_draw_command = video_register_draw_command,
};

