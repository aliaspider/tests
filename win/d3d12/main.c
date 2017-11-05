#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "d3d12_common.h"

#include "common.h"
#include "video.h"
#include "input.h"

struct Vertex
{
    float position[3];
    float color[4];
};
D3D12_VIEWPORT m_viewport;
D3D12_RECT m_scissorRect;
ID3D12Resource* m_renderTargets[2];
ID3D12CommandAllocator* m_commandAllocator;
ID3D12RootSignature* m_rootSignature;
ID3D12DescriptorHeap* m_rtvHeap;
ID3D12PipelineState* m_pipelineState;
ID3D12GraphicsCommandList* m_commandList;
UINT m_rtvDescriptorSize;

ID3D12Resource* m_vertexBuffer;
D3D12_VERTEX_BUFFER_VIEW m_vertexBufferView;


HANDLE m_fenceEvent;
ID3D12Fence* m_fence;
UINT64 m_fenceValue;


static void video_init()
{
   DEBUG_LINE();
   ID3D12Debug* debugController;
   D3D12_GetDebugInterface(&debugController);
   D3D12_EnableDebugLayer(debugController);

   IDXGIFactory1* factory;
   DXGI_CreateFactory(&factory);

   IDXGIAdapter1* adapter;
   ID3D12Device* device;
   int i;
   while(SUCCEEDED(DXGI_EnumAdapters(factory, i++, &adapter)))
   {
       if (SUCCEEDED(D3D12_CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1,&device)))
          break;
       DXGI_ReleaseAdapter(adapter);
   }

   ID3D12CommandQueue* command_queue;
   {
      D3D12_COMMAND_QUEUE_DESC desc =
      {
         .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
         .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
      };
      D3D12_CreateCommandQueue(device, &desc, &command_queue);
   }

   IDXGISwapChain3* swapchain;
   {
      DXGI_SWAP_CHAIN_DESC desc =
      {
         .BufferDesc.Width = video.screens[0].width,
         .BufferDesc.Height = video.screens[0].height,
         .BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
         .SampleDesc.Count = 1,
         .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
         .BufferCount = countof(m_renderTargets),
         .OutputWindow = video.screens[0].hwnd,
         .Windowed = true,
         .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD
      };
      DXGI_CreateSwapChain(factory, command_queue, &desc, &swapchain);
   }
   DXGI_MakeWindowAssociation(factory, video.screens[0].hwnd, DXGI_MWA_NO_ALT_ENTER);

   int frame_index = DXGI_GetCurrentBackBufferIndex(swapchain);
   DEBUG_INT(frame_index);








}

static void video_render()
{
//   DEBUG_LINE();


}

static void video_destroy()
{
   DEBUG_LINE();
}

static void video_register_draw_command(int screen_id, draw_command_t fn)
{
}

const video_t video_d3d12 =
{
   .init                  = video_init,
   .render                = video_render,
   .destroy               = video_destroy,
   .register_draw_command = video_register_draw_command,
};

