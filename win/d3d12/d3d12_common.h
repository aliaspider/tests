#pragma once

#ifdef __MINGW32__
#define __REQUIRED_RPCNDR_H_VERSION__ 475
#define _In_
#define _In_opt_
#define _Null_

#define _Out_writes_bytes_opt_(s)
#endif
//#define CINTERFACE
//#define COBJMACROS
#ifdef __MINGW32__
#include <um/d3d12.h>
#include <dxgi1_5.h>
#include <um/d3dcompiler.h>
#else
#include <d3d12.h>
#include <dxgi1_5.h>
#include <d3dcompiler.h>
#endif
#include "common.h"

#define __uuidof(type) &IID_##type

#define CHECK_AND_RETURN(x) HRESULT res = x;CHECK_WINERR(res);return res;

typedef D3D12_BLEND_DESC D3D12BlendDesc;
typedef D3D12_GRAPHICS_PIPELINE_STATE_DESC D3D12GraphicsPipelineStateDesc;


static inline HRESULT D3D12_CreateCommandQueue(ID3D12Device* device, const D3D12_COMMAND_QUEUE_DESC *desc, ID3D12CommandQueue **out)
{
   CHECK_AND_RETURN(device->lpVtbl->CreateCommandQueue(device, desc, __uuidof(ID3D12CommandQueue), (void**)out));
}

static inline HRESULT D3D12_GetDebugInterface(ID3D12Debug** out )
{
   CHECK_AND_RETURN(D3D12GetDebugInterface(__uuidof(ID3D12Debug), (void**)out));
}

static inline void D3D12_EnableDebugLayer(ID3D12Debug* debug)
{
   debug->lpVtbl->EnableDebugLayer(debug);
}

static inline HRESULT DXGI_CreateFactory(IDXGIFactory1** factory)
{
   CHECK_AND_RETURN(CreateDXGIFactory1(__uuidof(IDXGIFactory1), (void**)factory));
}
static inline HRESULT DXGI_EnumAdapters(IDXGIFactory1* factory, UINT id, IDXGIAdapter1 **out)
{
   return factory->lpVtbl->EnumAdapters1(factory, id, out);
}

static inline HRESULT D3D12_CreateDevice(IDXGIAdapter1* adapter, D3D_FEATURE_LEVEL MinimumFeatureLevel, ID3D12Device** out)
{
   return D3D12CreateDevice((IUnknown*)adapter, MinimumFeatureLevel, __uuidof(ID3D12Device), (void**)out);
}

static ULONG DXGI_ReleaseAdapter(IDXGIAdapter1* adapter)
{
   return adapter->lpVtbl->Release(adapter);
}

static inline HRESULT DXGI_CreateSwapChain(IDXGIFactory1* factory, ID3D12CommandQueue *queue, DXGI_SWAP_CHAIN_DESC *desc, IDXGISwapChain3 **swapchain)
{
   CHECK_AND_RETURN(factory->lpVtbl->CreateSwapChain(factory, (IUnknown*)queue, desc, (IDXGISwapChain**)swapchain));
}

static inline HRESULT DXGI_MakeWindowAssociation(IDXGIFactory1* factory, HWND window, UINT flags)
{
   CHECK_AND_RETURN(factory->lpVtbl->MakeWindowAssociation(factory, window, flags));

}

static inline UINT DXGI_GetCurrentBackBufferIndex(IDXGISwapChain3* swapchain)
{
   return swapchain->lpVtbl->GetCurrentBackBufferIndex(swapchain);
}


static inline HRESULT D3D12_CreateDescriptorHeap (ID3D12Device * device, const D3D12_DESCRIPTOR_HEAP_DESC *desc, ID3D12DescriptorHeap** out)
{
   CHECK_AND_RETURN(device->lpVtbl->CreateDescriptorHeap(device, desc, __uuidof(ID3D12DescriptorHeap), (void**)out));
}

static inline UINT D3D12_GetDescriptorHandleIncrementSize (ID3D12Device * device, D3D12_DESCRIPTOR_HEAP_TYPE type)
{
   return device->lpVtbl->GetDescriptorHandleIncrementSize(device, type);
}

static inline void D3D12_GetCPUDescriptorHandleForHeapStart(ID3D12DescriptorHeap * heap, D3D12_CPU_DESCRIPTOR_HANDLE* out)
{
   ((void (STDMETHODCALLTYPE *)(ID3D12DescriptorHeap*, D3D12_CPU_DESCRIPTOR_HANDLE*))
         heap->lpVtbl->GetCPUDescriptorHandleForHeapStart)(heap, out);
}

void* D3D_GetBufferPointer(ID3DBlob* buffer)
{
   return buffer->lpVtbl->GetBufferPointer(buffer);
}

size_t D3D_GetBufferSize(ID3DBlob* buffer)
{
   return buffer->lpVtbl->GetBufferSize(buffer);
}
