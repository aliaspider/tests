#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <d3d11.h>

#include "common.h"
#include "video.h"
#include "input.h"


static IDXGISwapChain *swapChain;
static ID3D11Device *device;
static D3D_FEATURE_LEVEL supportedFeatureLevel;
static ID3D11DeviceContext *context;
static ID3D11RenderTargetView *renderTargetView;


static ID3D11DeviceVtbl d3d;
static ID3D11DeviceContextVtbl ctx;
static IDXGISwapChainVtbl dxgi;



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
         .BufferDesc.RefreshRate.Numerator = 60,
         .BufferDesc.RefreshRate.Denominator = 1,
         .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
         .OutputWindow = video.screens[0].hwnd,
         .SampleDesc.Count = 1,
         .SampleDesc.Quality = 0,
         .Windowed = TRUE,
      };
      D3D_FEATURE_LEVEL requested_feature_level = D3D_FEATURE_LEVEL_11_0;

      CHECK_WINERR(D3D11CreateDeviceAndSwapChain(NULL, D3D_DRIVER_TYPE_HARDWARE, NULL,
#ifdef DEBUG
                   D3D11_CREATE_DEVICE_DEBUG,
#else
                   0,
#endif
                   &requested_feature_level, 1, D3D11_SDK_VERSION, &desc,
                   &swapChain, &device, &supportedFeatureLevel, &context));
   }

   d3d = *device->lpVtbl;
   ctx = *context->lpVtbl;
   dxgi = *swapChain->lpVtbl;

   ID3D11Texture2D *backBuffer;
   dxgi.GetBuffer(swapChain, 0, __uuidof(ID3D11Texture2D), (void **)&backBuffer);
   d3d.CreateRenderTargetView(device, (ID3D11Resource *)backBuffer, NULL, &renderTargetView);
   Release(backBuffer);
   ctx.OMSetRenderTargets(context, 1, &renderTargetView, NULL);

   D3D11_VIEWPORT vp = {0, 0, video.screens[0].width, video.screens[0].height, 0.0f, 1.0f};
   ctx.RSSetViewports(context, 1, &vp);

}


static void video_render()
{

}

static void video_destroy()
{
   DEBUG_LINE();
}

static void video_register_draw_command(int screen_id, draw_command_t fn)
{
}


const video_t video_d3d11 =
{
   .init                  = video_init,
   .render                = video_render,
   .destroy               = video_destroy,
   .register_draw_command = video_register_draw_command,
};

