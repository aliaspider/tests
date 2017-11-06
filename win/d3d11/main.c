#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#ifdef __MINGW32__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#define _Null_
#endif
#include <um/d3dcommon.h>
#include <d3d11.h>
#include <dxgi1_5.h>
#include <um/d3dcompiler.h>

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


static inline void *D3D_GetBufferPointer(ID3DBlob *buffer)
{
   return buffer->lpVtbl->GetBufferPointer(buffer);
}

static inline size_t D3D_GetBufferSize(ID3DBlob *buffer)
{
   return buffer->lpVtbl->GetBufferSize(buffer);
}


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

   typedef struct Vertex
   {
      float position[3];
      float color[4];
   } Vertex;

   Vertex vertices [3] =
   {
      {{-1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
      {{0.0f, 1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
      {{1.0f, -1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
   };
   D3D11_BUFFER_DESC vertexBufferDesc =
   {
      .Usage = D3D11_USAGE_DEFAULT,
      .ByteWidth = sizeof(vertices),
      .BindFlags = D3D11_BIND_VERTEX_BUFFER,
      .StructureByteStride = 0, /* sizeof(Vertex) ? */
   };
   D3D11_SUBRESOURCE_DATA vertexData = {vertices};

   ID3D11Buffer *vertexBuffer;
   CHECK_WINERR(d3d.CreateBuffer(device, &vertexBufferDesc, &vertexData, &vertexBuffer));
   {
      UINT stride = sizeof(Vertex);
      UINT offset = 0;
      ctx.IASetVertexBuffers(context, 0, 1, &vertexBuffer, &stride, &offset);
   }
   ctx.IASetPrimitiveTopology(context, D3D11_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   ID3D11VertexShader *vs;
   ID3D11PixelShader *ps;
   ID3D11InputLayout *layout;

   ID3DBlob *vs_code;
   ID3DBlob *ps_code;
#ifdef DEBUG
   UINT compileflags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
   UINT compileflags = 0;
#endif
   {
      ID3DBlob *error_msg;
      CHECK_WINERR(D3DCompileFromFile(L"win/d3d11/shaders.hlsl", NULL, NULL, "VSMain", "vs_5_0",
                                      compileflags, 0, &vs_code, &error_msg));

      if (error_msg)
      {
         printf("D3DCompileFromFile failed :\n%s\n", (const char *)D3D_GetBufferPointer(error_msg));
         fflush(stdout);
         Release(error_msg);
      }

      CHECK_WINERR(D3DCompileFromFile(L"win/d3d11/shaders.hlsl", NULL, NULL, "PSMain", "ps_5_0",
                                      compileflags, 0, &ps_code, &error_msg));

      if (error_msg)
      {
         printf("D3DCompileFromFile failed :\n%s\n", (const char *)D3D_GetBufferPointer(error_msg));
         fflush(stdout);
         Release(error_msg);
      }
   }

   D3D11_INPUT_ELEMENT_DESC inputElementDesc[] =
   {
      {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex, position), D3D11_INPUT_PER_VERTEX_DATA, 0},
      {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color),    D3D11_INPUT_PER_VERTEX_DATA, 0},
   };

   d3d.CreateVertexShader(device, D3D_GetBufferPointer(vs_code), D3D_GetBufferSize(vs_code), NULL,
                          &vs);
   d3d.CreatePixelShader(device, D3D_GetBufferPointer(ps_code), D3D_GetBufferSize(ps_code), NULL, &ps);
   d3d.CreateInputLayout(device, inputElementDesc, countof(inputElementDesc),
                         D3D_GetBufferPointer(vs_code), D3D_GetBufferSize(vs_code), &layout);
   Release(vs_code);
   Release(ps_code);

   ctx.IASetInputLayout(context, layout);
   ctx.VSSetShader(context, vs, NULL, 0);
   ctx.PSSetShader(context, ps, NULL, 0);

}


static void video_render()
{
   float clearcolor[4] = {0.5, 0.2, 1.0, 1.0};
   ctx.ClearRenderTargetView(context, renderTargetView, clearcolor);

   ctx.Draw(context, 3, 0);
   dxgi.Present(swapChain, 0, 0);



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

