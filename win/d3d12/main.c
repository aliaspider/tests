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

typedef ID3D12Debug* D3D12Debug;
typedef IDXGIFactory1* DXGIFactory;
typedef IDXGIAdapter1* DXGIAdapter;
typedef ID3D12Device* D3D12Device;
typedef ID3D12CommandQueue* D3D12CommandQueue;
typedef IDXGISwapChain3* DXGISwapChain;
typedef ID3D12Resource* D3D12Resource;
typedef ID3D12DescriptorHeap* D3D12DescriptorHeap;
typedef ID3D12CommandAllocator* D3D12CommandAllocator;
typedef ID3D12RootSignature* D3D12RootSignature;
typedef ID3D12PipelineState* D3D12PipelineState;
typedef ID3D12GraphicsCommandList* D3D12GraphicsCommandList;
typedef ID3D12Resource* D3D12Resource;
typedef ID3D12Fence* D3D12Fence;


D3D12Debug debugController;
DXGIFactory factory;
DXGIAdapter adapter;
D3D12Device device;
D3D12CommandQueue command_queue;
DXGISwapChain swapchain;
int frame_index;
D3D12Resource renderTargets[2];
D3D12DescriptorHeap rtvHeap;
UINT rtvDescriptorSize;
D3D12CommandAllocator commandAllocator;
D3D12RootSignature rootSignature;
D3D12PipelineState pipelineState;
D3D12GraphicsCommandList commandList;
D3D12Resource vertexBuffer;
D3D12Fence fence;
HANDLE fenceEvent;
UINT64 fenceValue;
D3D12_VERTEX_BUFFER_VIEW vertexBufferView;
D3D12_VIEWPORT viewport;
D3D12_RECT scissorRect;

static void video_init()
{

   DEBUG_LINE();
   D3D12_GetDebugInterface(&debugController);
   D3D12_EnableDebugLayer(debugController);

   DXGI_CreateFactory(&factory);

   int i = 0;
   while(SUCCEEDED(DXGI_EnumAdapters(factory, i++, &adapter)))
   {
       if (SUCCEEDED(D3D12_CreateDevice(adapter, D3D_FEATURE_LEVEL_12_1,&device)))
          break;
       DXGI_ReleaseAdapter(adapter);
   }

   {
      D3D12_COMMAND_QUEUE_DESC desc =
      {
         .Type = D3D12_COMMAND_LIST_TYPE_DIRECT,
         .Flags = D3D12_COMMAND_QUEUE_FLAG_NONE,
      };
      D3D12_CreateCommandQueue(device, &desc, &command_queue);
   }

   {
      DXGI_SWAP_CHAIN_DESC desc =
      {
         .BufferDesc.Width = video.screens[0].width,
         .BufferDesc.Height = video.screens[0].height,
         .BufferDesc.Format = DXGI_FORMAT_R8G8B8A8_UNORM,
         .SampleDesc.Count = 1,
         .BufferUsage = DXGI_USAGE_RENDER_TARGET_OUTPUT,
         .BufferCount = countof(renderTargets),
         .OutputWindow = video.screens[0].hwnd,
         .Windowed = true,
         .SwapEffect = DXGI_SWAP_EFFECT_FLIP_DISCARD
      };
      DXGI_CreateSwapChain(factory, command_queue, &desc, &swapchain);
   }
   DXGI_MakeWindowAssociation(factory, video.screens[0].hwnd, DXGI_MWA_NO_ALT_ENTER);

   frame_index = DXGI_GetCurrentBackBufferIndex(swapchain);

   {
      D3D12_DESCRIPTOR_HEAP_DESC desc =
      {
         .Type = D3D12_DESCRIPTOR_HEAP_TYPE_RTV,
         .NumDescriptors = countof(renderTargets),
         .Flags = D3D12_DESCRIPTOR_HEAP_FLAG_NONE,
      };
      D3D12_CreateDescriptorHeap(device, &desc, &rtvHeap);
   }
   rtvDescriptorSize = D3D12_GetDescriptorHandleIncrementSize(device, D3D12_DESCRIPTOR_HEAP_TYPE_RTV);

   {
      D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
      D3D12_GetCPUDescriptorHandleForHeapStart(rtvHeap, &rtvHandle);

      for(int i= 0; i < countof(renderTargets); i++)
      {
         CHECK_WINERR(swapchain->lpVtbl->GetBuffer(swapchain, i, __uuidof(ID3D12Resource), (void**)&renderTargets[i]));
         device->lpVtbl->CreateRenderTargetView(device, renderTargets[i], NULL, rtvHandle);
         rtvHandle.ptr += rtvDescriptorSize;
      }
   }

   CHECK_WINERR(device->lpVtbl->CreateCommandAllocator(device, D3D12_COMMAND_LIST_TYPE_DIRECT, __uuidof(ID3D12CommandAllocator), (void**)&commandAllocator));

   {
      D3D12_ROOT_SIGNATURE_DESC desc =
      {
         .NumParameters = 0,
         .pParameters = NULL,
         .NumStaticSamplers = 0,
         .pStaticSamplers = NULL,
         .Flags = D3D12_ROOT_SIGNATURE_FLAG_ALLOW_INPUT_ASSEMBLER_INPUT_LAYOUT,
      };

      ID3DBlob* signature;
      ID3DBlob* error;
      CHECK_WINERR(D3D12SerializeRootSignature(&desc, D3D_ROOT_SIGNATURE_VERSION_1, &signature, &error));
      CHECK_WINERR(device->lpVtbl->CreateRootSignature(device, 0, signature->lpVtbl->GetBufferPointer(signature), signature->lpVtbl->GetBufferSize(signature),
                                                       __uuidof(ID3D12RootSignature), (void**)&rootSignature));

      signature->lpVtbl->Release(signature);
      if(error)
         error->lpVtbl->Release(error);
   }

   typedef struct Vertex
   {
       float position[3];
       float color[4];
   }Vertex;

   Vertex vertices [] =
   {
      { { 0.0f, 0.25f , 0.0f }, { 1.0f, 0.0f, 0.0f, 1.0f } },
      { { 0.25f, -0.25f , 0.0f }, { 0.0f, 1.0f, 0.0f, 1.0f } },
      { { -0.25f, -0.25f , 0.0f }, { 0.0f, 0.0f, 1.0f, 1.0f } }
   };

   {
      ID3DBlob* vertexshader;
      ID3DBlob* pixelshader;
#ifdef DEBUG
      UINT compileflags = D3DCOMPILE_DEBUG | D3DCOMPILE_SKIP_OPTIMIZATION;
#else
      UINT compileflags = 0;
#endif
      CHECK_WINERR(D3DCompileFromFile(L"win/d3d12/shaders.hlsl", NULL, NULL, "VSMain", "vs_5_0", compileflags, 0, &vertexshader, NULL));
      CHECK_WINERR(D3DCompileFromFile(L"win/d3d12/shaders.hlsl", NULL, NULL, "PSMain", "ps_5_0", compileflags, 0, &pixelshader, NULL));

      D3D12_INPUT_ELEMENT_DESC inputElementDesc[] =
      {
         {"POSITION", 0, DXGI_FORMAT_R32G32B32_FLOAT,    0, offsetof(Vertex, position), D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
         {"COLOR",    0, DXGI_FORMAT_R32G32B32A32_FLOAT, 0, offsetof(Vertex, color),    D3D12_INPUT_CLASSIFICATION_PER_VERTEX_DATA, 0},
      };

      D3D12_RASTERIZER_DESC rasterizerDesc =
      {
         .FillMode = D3D12_FILL_MODE_SOLID,
         .CullMode = D3D12_CULL_MODE_BACK,
         .FrontCounterClockwise = FALSE,
         .DepthBias = D3D12_DEFAULT_DEPTH_BIAS,
         .DepthBiasClamp = D3D12_DEFAULT_DEPTH_BIAS_CLAMP,
         .SlopeScaledDepthBias = D3D12_DEFAULT_SLOPE_SCALED_DEPTH_BIAS,
         .DepthClipEnable = TRUE,
         .MultisampleEnable = FALSE,
         .AntialiasedLineEnable = FALSE,
         .ForcedSampleCount = 0,
         .ConservativeRaster = D3D12_CONSERVATIVE_RASTERIZATION_MODE_OFF,
      };

      const D3D12_RENDER_TARGET_BLEND_DESC defaultRenderTargetBlendDesc =
      {
         .BlendEnable = FALSE, .LogicOpEnable = FALSE,
         D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
         D3D12_BLEND_ONE, D3D12_BLEND_ZERO, D3D12_BLEND_OP_ADD,
         D3D12_LOGIC_OP_NOOP,
         D3D12_COLOR_WRITE_ENABLE_ALL,
      };

      D3D12BlendDesc blendDesc =
      {
         .AlphaToCoverageEnable = FALSE,
         .IndependentBlendEnable = FALSE,
         .RenderTarget[0] = defaultRenderTargetBlendDesc,
      };

      D3D12GraphicsPipelineStateDesc psodesc =
      {
         .pRootSignature = rootSignature,
         .VS.pShaderBytecode = D3D_GetBufferPointer(vertexshader), D3D_GetBufferSize(vertexshader),
         .PS.pShaderBytecode = D3D_GetBufferPointer(pixelshader), D3D_GetBufferSize(pixelshader),
         .BlendState = blendDesc,
         .SampleMask = UINT_MAX,
         .RasterizerState = rasterizerDesc,
         .DepthStencilState.DepthEnable = FALSE,
         .DepthStencilState.StencilEnable = FALSE,
         .InputLayout.pInputElementDescs = inputElementDesc, countof(inputElementDesc),
         .PrimitiveTopologyType = D3D12_PRIMITIVE_TOPOLOGY_TYPE_TRIANGLE,
         .NumRenderTargets = 1,
         .RTVFormats[0] = DXGI_FORMAT_R8G8B8A8_UNORM,
         .SampleDesc.Count = 1,
      };

      device->lpVtbl->CreateGraphicsPipelineState(device, &psodesc, __uuidof(ID3D12PipelineState), (void**)&pipelineState);
   }

   device->lpVtbl->CreateCommandList(device, 0, D3D12_COMMAND_LIST_TYPE_DIRECT, commandAllocator, pipelineState,
                                     __uuidof(ID3D12GraphicsCommandList), (void**)&commandList);
   commandList->lpVtbl->Close(commandList);

   {
      D3D12_HEAP_PROPERTIES heap_props =
      {
         .Type = D3D12_HEAP_TYPE_UPLOAD,
         .CPUPageProperty = D3D12_CPU_PAGE_PROPERTY_UNKNOWN,
         .MemoryPoolPreference = D3D12_MEMORY_POOL_UNKNOWN,
         .CreationNodeMask = 1,
         .VisibleNodeMask = 1,
      };

      D3D12_RESOURCE_DESC resource_desc =
      {
         .Dimension = D3D12_RESOURCE_DIMENSION_BUFFER,
         .Alignment = 0,
         .Width = sizeof(vertices),
         .Height = 1,
         .DepthOrArraySize = 1,
         .MipLevels = 1,
         .Format = DXGI_FORMAT_UNKNOWN,
         .SampleDesc.Count = 1,
         .SampleDesc.Quality = 0,
         .Layout = D3D12_TEXTURE_LAYOUT_ROW_MAJOR,
         .Flags = D3D12_RESOURCE_FLAG_NONE,
      };



      device->lpVtbl->CreateCommittedResource(device, &heap_props, D3D12_HEAP_FLAG_NONE,
                                              &resource_desc, D3D12_RESOURCE_STATE_GENERIC_READ,
                                              NULL, __uuidof(ID3D12Resource), (void**)&vertexBuffer);
      uint8_t* vertex_data_begin;
      D3D12_RANGE read_range = {0, 0};
      CHECK_WINERR(vertexBuffer->lpVtbl->Map(vertexBuffer, 0, &read_range, (void**)&vertex_data_begin));
      memcpy(vertex_data_begin, vertices, sizeof(vertices));
      vertexBuffer->lpVtbl->Unmap(vertexBuffer, 0, NULL);

   }

   vertexBufferView.BufferLocation = vertexBuffer->lpVtbl->GetGPUVirtualAddress(vertexBuffer);
   vertexBufferView.SizeInBytes = sizeof(vertices);
   vertexBufferView.StrideInBytes = sizeof(Vertex);

   CHECK_WINERR(device->lpVtbl->CreateFence(device, 0, D3D12_FENCE_FLAG_NONE, __uuidof(ID3D12Fence), (void**)&fence));
   fenceValue = 1;
   fenceEvent = CreateEvent(NULL, FALSE, FALSE, NULL);

   viewport.Width = video.screens[0].width;
   viewport.Height = video.screens[0].height;
   scissorRect.right = video.screens[0].width;
   scissorRect.bottom = video.screens[0].height;


}

void video_wait_for_previous_frame(void)
{
	CHECK_WINERR(command_queue->lpVtbl->Signal(command_queue, fence, fenceValue));

	// Wait until the previous frame is finished.
	if (fence->lpVtbl->GetCompletedValue(fence) < fenceValue)
	{
		CHECK_WINERR(fence->lpVtbl->SetEventOnCompletion(fence, fenceValue, fenceEvent));
		WaitForSingleObject(fenceEvent, INFINITE);
	}

	frame_index = swapchain->lpVtbl->GetCurrentBackBufferIndex(swapchain);

   fenceValue++;
}

static void video_render()
{
//   DEBUG_LINE();
//   fflush(stdout);

   CHECK_WINERR(commandAllocator->lpVtbl->Reset(commandAllocator));
   CHECK_WINERR(commandList->lpVtbl->Reset(commandList, commandAllocator, pipelineState));
   commandList->lpVtbl->SetGraphicsRootSignature(commandList, rootSignature);
   commandList->lpVtbl->RSSetViewports(commandList, 1, &viewport);
   commandList->lpVtbl->RSSetScissorRects(commandList, 1, &scissorRect);

   D3D12_RESOURCE_BARRIER barrier =
   {
      .Type = D3D12_RESOURCE_BARRIER_TYPE_TRANSITION,
      .Flags = D3D12_RESOURCE_BARRIER_FLAG_NONE,
      .Transition.pResource = renderTargets[frame_index],
      .Transition.StateBefore = D3D12_RESOURCE_STATE_PRESENT,
      .Transition.StateAfter = D3D12_RESOURCE_STATE_RENDER_TARGET,
      .Transition.Subresource = D3D12_RESOURCE_BARRIER_ALL_SUBRESOURCES,
   };
   commandList->lpVtbl->ResourceBarrier(commandList, 1, &barrier);
   D3D12_CPU_DESCRIPTOR_HANDLE rtvHandle;
   D3D12_GetCPUDescriptorHandleForHeapStart(rtvHeap, &rtvHandle);
   rtvHandle.ptr += frame_index *  rtvDescriptorSize;
   commandList->lpVtbl->OMSetRenderTargets(commandList, 1, &rtvHandle, FALSE, NULL);

   static const float clearcolor[] = {0.0, 0.2, 0.4, 1.0};
   commandList->lpVtbl->ClearRenderTargetView(commandList, rtvHandle, clearcolor, 0, NULL);
   commandList->lpVtbl->IASetPrimitiveTopology(commandList, D3D_PRIMITIVE_TOPOLOGY_TRIANGLELIST);
   commandList->lpVtbl->IASetVertexBuffers(commandList, 0, 1, &vertexBufferView);
   commandList->lpVtbl->DrawInstanced(commandList, 3, 1, 0, 0);

   barrier.Transition.StateBefore = D3D12_RESOURCE_STATE_RENDER_TARGET;
   barrier.Transition.StateAfter = D3D12_RESOURCE_STATE_PRESENT;

   commandList->lpVtbl->ResourceBarrier(commandList, 1, &barrier);
   CHECK_WINERR(commandList->lpVtbl->Close(commandList));





   command_queue->lpVtbl->ExecuteCommandLists(command_queue, 1, (ID3D12CommandList**)&commandList);
   swapchain->lpVtbl->Present(swapchain, 0, 0);

   video_wait_for_previous_frame();
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

