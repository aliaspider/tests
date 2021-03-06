//#pragma once

//#include <d3d9.h>
////#include <d3dcompiler.h>
//#ifdef __MINGW32__
//#define __REQUIRED_RPCNDR_H_VERSION__ 475
//#define _In_
//#define _In_opt_
//#define _Null_
//#define _Out_writes_bytes_opt_(s)
//#endif
//#include <um/d3dcompiler.h>
//#include "common.h"

#define CNT_ARGS(...) CNT_ARGS_(__VA_ARGS__,24,23,22,21,20,19,18,17,16,15,14,13,12,11,10,9,8,7,6,5,4,3,2,1,0)
#define CNT_ARGS_(_1,_2,_3,_4,_5,_6,_7,_8,_9,_10,_11,_12,_13,_14,_15,_16,_17,_18,_19,_20,_21,_22,_23,_24,n,...) n


#define DROP_TYPE(...) DROP_TYPE_(CNT_ARGS(__VA_ARGS__),__VA_ARGS__)
#define DROP_TYPE_(n,...) DROP_TYPE__(n,__VA_ARGS__)
#define DROP_TYPE__(n,...) DROP_TYPE_##n(__VA_ARGS__)
#define DROP_TYPE_1()
#define DROP_TYPE_2(ptype,pname) ,pname
#define DROP_TYPE_4(ptype,pname,...) ,pname DROP_TYPE_2(__VA_ARGS__)
#define DROP_TYPE_6(ptype,pname,...) ,pname DROP_TYPE_4(__VA_ARGS__)
#define DROP_TYPE_8(ptype,pname,...) ,pname DROP_TYPE_6(__VA_ARGS__)
#define DROP_TYPE_10(ptype,pname,...) ,pname DROP_TYPE_8(__VA_ARGS__)
#define DROP_TYPE_12(ptype,pname,...) ,pname DROP_TYPE_10(__VA_ARGS__)
#define DROP_TYPE_14(ptype,pname,...) ,pname DROP_TYPE_12(__VA_ARGS__)
#define DROP_TYPE_16(ptype,pname,...) ,pname DROP_TYPE_14(__VA_ARGS__)
#define DROP_TYPE_18(ptype,pname,...) ,pname DROP_TYPE_16(__VA_ARGS__)

#define MERGE_TYPE(...) MERGE_TYPE_(CNT_ARGS(__VA_ARGS__),__VA_ARGS__)
#define MERGE_TYPE_(n,...) MERGE_TYPE__(n,__VA_ARGS__)
#define MERGE_TYPE__(n,...) MERGE_TYPE_##n(__VA_ARGS__)
#define MERGE_TYPE_1()
#define MERGE_TYPE_2(ptype,pname) ,ptype pname
#define MERGE_TYPE_4(ptype,pname,...) ,ptype pname MERGE_TYPE_2(__VA_ARGS__)
#define MERGE_TYPE_6(ptype,pname,...) ,ptype pname MERGE_TYPE_4(__VA_ARGS__)
#define MERGE_TYPE_8(ptype,pname,...) ,ptype pname MERGE_TYPE_6(__VA_ARGS__)
#define MERGE_TYPE_10(ptype,pname,...) ,ptype pname MERGE_TYPE_8(__VA_ARGS__)
#define MERGE_TYPE_12(ptype,pname,...) ,ptype pname MERGE_TYPE_10(__VA_ARGS__)
#define MERGE_TYPE_14(ptype,pname,...) ,ptype pname MERGE_TYPE_12(__VA_ARGS__)
#define MERGE_TYPE_16(ptype,pname,...) ,ptype pname MERGE_TYPE_14(__VA_ARGS__)
#define MERGE_TYPE_18(ptype,pname,...) ,ptype pname MERGE_TYPE_16(__VA_ARGS__)

#define CONCAT_(a,b) a##b
#define CONCAT(a,b) CONCAT_(a,b)

#define WRAP_(type,fn,...) static inline void CONCAT(PREFIX__,fn) (TYPE__ THIS__ MERGE_TYPE(__VA_ARGS__)) {CHECK_WINERR(THIS__->lpVtbl->fn(THIS__ DROP_TYPE( __VA_ARGS__)));}
#define WRAP(...) WRAP_(HRESULT, __VA_ARGS__)

#define WRAPNOCHECK_(type,fn,...) static inline type CONCAT(PREFIX__,fn) (TYPE__ THIS__ MERGE_TYPE(__VA_ARGS__)) {return THIS__->lpVtbl->fn(THIS__ DROP_TYPE( __VA_ARGS__));}
#define WRAPNOCHECK(...) WRAPNOCHECK_(HRESULT, __VA_ARGS__)

#define IUNKNOWN__ \
   WRAP_(HRESULT, QueryInterface,  REFIID, riid, void**, Object) \
   WRAP_(ULONG, AddRef) \
   WRAP_(ULONG, Release) \

typedef IDirect3D9* Direct3D9;
typedef IDirect3DDevice9* Direct3DDevice9;
typedef IDirect3DVertexBuffer9* Direct3DVertexBuffer9;
typedef IDirect3DVertexShader9* Direct3DVertexShader9;
typedef IDirect3DPixelShader9* Direct3DPixelShader9;
typedef ID3DBlob* D3DBlob;

#define DWORD u32
#define UINT u32

#define THIS__ d3d
#define TYPE__ Direct3D9
#define PREFIX__ D3D9_
IUNKNOWN__
WRAP(RegisterSoftwareDevice, void*, pInitializeFunction)
WRAP_(UINT, GetAdapterCount)
WRAP(GetAdapterIdentifier, UINT, Adapter, DWORD, Flags, D3DADAPTER_IDENTIFIER9*, pIdentifier)
WRAP_(UINT, GetAdapterModeCount, UINT, Adapter, D3DFORMAT, Format)
WRAP(EnumAdapterModes, UINT, Adapter, D3DFORMAT, Format, UINT, Mode, D3DDISPLAYMODE*, pMode)
WRAP(GetAdapterDisplayMode, UINT, Adapter, D3DDISPLAYMODE*, pMode)
WRAP(CheckDeviceType, UINT, iAdapter, D3DDEVTYPE, DevType, D3DFORMAT, DisplayFormat, D3DFORMAT, BackBufferFormat, WINBOOL, bWindowed)
WRAP(CheckDeviceFormat, UINT, Adapter, D3DDEVTYPE, DeviceType, D3DFORMAT, AdapterFormat, DWORD, Usage, D3DRESOURCETYPE, RType, D3DFORMAT, CheckFormat)
WRAP(CheckDeviceMultiSampleType, UINT, Adapter, D3DDEVTYPE, DeviceType, D3DFORMAT, SurfaceFormat, WINBOOL, Windowed, D3DMULTISAMPLE_TYPE, MultiSampleType, DWORD*, pQualityLevels)
WRAP(CheckDepthStencilMatch, UINT, Adapter, D3DDEVTYPE, DeviceType, D3DFORMAT, AdapterFormat, D3DFORMAT, RenderTargetFormat, D3DFORMAT, DepthStencilFormat)
WRAP(CheckDeviceFormatConversion, UINT, Adapter, D3DDEVTYPE, DeviceType, D3DFORMAT, SourceFormat, D3DFORMAT, TargetFormat)
WRAP(GetDeviceCaps, UINT, Adapter, D3DDEVTYPE, DeviceType, D3DCAPS9*, pCaps)
WRAPNOCHECK_(HMONITOR, GetAdapterMonitor, UINT, Adapter)
WRAP(CreateDevice, UINT, Adapter, D3DDEVTYPE, DeviceType, HWND, hFocusWindow, DWORD, BehaviorFlags, D3DPRESENT_PARAMETERS*, pPresentationParameters, struct IDirect3DDevice9**, ppReturnedDeviceInterface)
#undef THIS__
#undef TYPE__
#undef PREFIX__


#define THIS__ device
#define TYPE__ Direct3DDevice9
#define PREFIX__ D3D9_
//IUNKNOWN__
WRAP(TestCooperativeLevel)
WRAP_(UINT, GetAvailableTextureMem)
WRAP(EvictManagedResources)
WRAP(GetDirect3D, IDirect3D9**, ppD3D9)
//WRAP(GetDeviceCaps, D3DCAPS9*, pCaps)
WRAP(GetDisplayMode, UINT, iSwapChain, D3DDISPLAYMODE*, pMode)
WRAP(GetCreationParameters, D3DDEVICE_CREATION_PARAMETERS *,pParameters)
WRAP(SetCursorProperties, UINT, XHotSpot, UINT, YHotSpot, IDirect3DSurface9*, pCursorBitmap)
WRAPNOCHECK_(void, SetCursorPosition, int, X,int, Y, DWORD, Flags)
WRAP_(WINBOOL, ShowCursor, WINBOOL, bShow)
WRAP(CreateAdditionalSwapChain, D3DPRESENT_PARAMETERS*, pPresentationParameters, IDirect3DSwapChain9**, pSwapChain)
WRAP(GetSwapChain, UINT, iSwapChain, IDirect3DSwapChain9**, pSwapChain)
WRAP_(UINT, GetNumberOfSwapChains)
WRAP(Reset, D3DPRESENT_PARAMETERS*, pPresentationParameters)
WRAP(Present, const RECT *,src_rect, const RECT *,dst_rect, HWND, dst_window_override, const RGNDATA *,dirty_region)
WRAP(GetBackBuffer, UINT, iSwapChain, UINT, iBackBuffer, D3DBACKBUFFER_TYPE, Type, IDirect3DSurface9**, ppBackBuffer)
WRAP(GetRasterStatus, UINT, iSwapChain, D3DRASTER_STATUS*, pRasterStatus)
WRAP(SetDialogBoxMode, WINBOOL, bEnableDialogs)
WRAPNOCHECK_(void, SetGammaRamp, UINT, swapchain_idx, DWORD, flags, const D3DGAMMARAMP *,ramp)
WRAPNOCHECK_(void, GetGammaRamp, UINT, iSwapChain, D3DGAMMARAMP*, pRamp)
WRAP(CreateTexture, UINT, Width, UINT, Height, UINT, Levels, DWORD, Usage, D3DFORMAT, Format, D3DPOOL, Pool, IDirect3DTexture9**, ppTexture, HANDLE*, pSharedHandle)
WRAP(CreateVolumeTexture, UINT, Width, UINT, Height, UINT, Depth, UINT, Levels, DWORD, Usage, D3DFORMAT, Format, D3DPOOL, Pool, IDirect3DVolumeTexture9**, ppVolumeTexture, HANDLE*, pSharedHandle)
WRAP(CreateCubeTexture, UINT, EdgeLength, UINT, Levels, DWORD, Usage, D3DFORMAT, Format, D3DPOOL, Pool, IDirect3DCubeTexture9**, ppCubeTexture, HANDLE*, pSharedHandle)
WRAP(CreateVertexBuffer, UINT, Length, DWORD, Usage, DWORD, FVF, D3DPOOL, Pool, IDirect3DVertexBuffer9**, ppVertexBuffer, HANDLE*, pSharedHandle)
WRAP(CreateIndexBuffer, UINT, Length, DWORD, Usage, D3DFORMAT, Format, D3DPOOL, Pool, IDirect3DIndexBuffer9**, ppIndexBuffer, HANDLE*, pSharedHandle)
WRAP(CreateRenderTarget, UINT, Width, UINT, Height, D3DFORMAT, Format, D3DMULTISAMPLE_TYPE, MultiSample, DWORD, MultisampleQuality, WINBOOL, Lockable, IDirect3DSurface9**, ppSurface, HANDLE*, pSharedHandle)
WRAP(CreateDepthStencilSurface, UINT, Width, UINT, Height, D3DFORMAT, Format, D3DMULTISAMPLE_TYPE, MultiSample, DWORD, MultisampleQuality, WINBOOL, Discard, IDirect3DSurface9**, ppSurface, HANDLE*, pSharedHandle)
WRAP(UpdateSurface, IDirect3DSurface9 *,src_surface, const RECT *,src_rect, IDirect3DSurface9 *,dst_surface, const POINT *,dst_point)
WRAP(UpdateTexture, IDirect3DBaseTexture9*, pSourceTexture, IDirect3DBaseTexture9*, pDestinationTexture)
WRAP(GetRenderTargetData, IDirect3DSurface9*, pRenderTarget, IDirect3DSurface9*, pDestSurface)
WRAP(GetFrontBufferData, UINT, iSwapChain, IDirect3DSurface9*, pDestSurface)
WRAP(StretchRect, IDirect3DSurface9 *,src_surface, const RECT *,src_rect, IDirect3DSurface9 *,dst_surface, const RECT *,dst_rect, D3DTEXTUREFILTERTYPE, filter)
WRAP(ColorFill, IDirect3DSurface9 *,surface, const RECT *,rect, D3DCOLOR, color)
WRAP(CreateOffscreenPlainSurface, UINT, Width, UINT, Height, D3DFORMAT, Format, D3DPOOL, Pool, IDirect3DSurface9**, ppSurface, HANDLE*, pSharedHandle)
WRAP(SetRenderTarget, DWORD, RenderTargetIndex, IDirect3DSurface9*, pRenderTarget)
WRAP(GetRenderTarget, DWORD, RenderTargetIndex, IDirect3DSurface9**, ppRenderTarget)
WRAP(SetDepthStencilSurface, IDirect3DSurface9*, pNewZStencil)
WRAP(GetDepthStencilSurface, IDirect3DSurface9**, ppZStencilSurface)
WRAP(BeginScene)
WRAP(EndScene)
WRAP(Clear, DWORD, rect_count, const D3DRECT *,rects, DWORD, flags,D3DCOLOR, color, float, z, DWORD, stencil)
WRAP(SetTransform, D3DTRANSFORMSTATETYPE, state, const D3DMATRIX *,matrix)
WRAP(GetTransform, D3DTRANSFORMSTATETYPE, State, D3DMATRIX*, pMatrix)
WRAP(MultiplyTransform, D3DTRANSFORMSTATETYPE, state, const D3DMATRIX *,matrix)
WRAP(SetViewport, const D3DVIEWPORT9 *,viewport)
WRAP(GetViewport, D3DVIEWPORT9*, pViewport)
WRAP(SetMaterial, const D3DMATERIAL9 *,material)
WRAP(GetMaterial, D3DMATERIAL9*, pMaterial)
WRAP(SetLight, DWORD, index, const D3DLIGHT9 *,light)
WRAP(GetLight, DWORD, Index, D3DLIGHT9*, light)
WRAP(LightEnable, DWORD, Index, WINBOOL, Enable)
WRAP(GetLightEnable, DWORD, Index, WINBOOL*, pEnable)
WRAP(SetClipPlane, DWORD, index, const float *,plane)
WRAP(GetClipPlane, DWORD, Index, float*, pPlane)
WRAP(SetRenderState, D3DRENDERSTATETYPE, State, DWORD, Value)
WRAP(GetRenderState, D3DRENDERSTATETYPE, State, DWORD*, pValue)
WRAP(CreateStateBlock, D3DSTATEBLOCKTYPE, Type, IDirect3DStateBlock9**, ppSB)
WRAP(BeginStateBlock)
WRAP(EndStateBlock, IDirect3DStateBlock9**, ppSB)
WRAP(SetClipStatus, const D3DCLIPSTATUS9 *,clip_status)
WRAP(GetClipStatus, D3DCLIPSTATUS9*, pClipStatus)
WRAP(GetTexture, DWORD, Stage, IDirect3DBaseTexture9**, ppTexture)
WRAP(SetTexture, DWORD, Stage, IDirect3DBaseTexture9*, pTexture)
WRAP(GetTextureStageState, DWORD, Stage, D3DTEXTURESTAGESTATETYPE, Type, DWORD*, pValue)
WRAP(SetTextureStageState, DWORD, Stage, D3DTEXTURESTAGESTATETYPE, Type, DWORD, Value)
WRAP(GetSamplerState, DWORD, Sampler, D3DSAMPLERSTATETYPE, Type, DWORD*, pValue)
WRAP(SetSamplerState, DWORD, Sampler, D3DSAMPLERSTATETYPE, Type, DWORD, Value)
WRAP(ValidateDevice, DWORD*, pNumPasses)
WRAP(SetPaletteEntries, UINT, palette_idx, const PALETTEENTRY *,entries)
WRAP(GetPaletteEntries, UINT, PaletteNumber,PALETTEENTRY*, pEntries)
WRAP(SetCurrentTexturePalette, UINT, PaletteNumber)
WRAP(GetCurrentTexturePalette, UINT*,PaletteNumber)
WRAP(SetScissorRect, const RECT *,rect)
WRAP(GetScissorRect, RECT*, pRect)
WRAP(SetSoftwareVertexProcessing, WINBOOL, bSoftware)
WRAP_(WINBOOL, GetSoftwareVertexProcessing)
WRAP(SetNPatchMode, float, nSegments)
WRAP_(float, GetNPatchMode)
WRAP(DrawPrimitive, D3DPRIMITIVETYPE, PrimitiveType, UINT, StartVertex, UINT, PrimitiveCount)
WRAP(DrawIndexedPrimitive, D3DPRIMITIVETYPE, PrimitiveType, INT, BaseVertexIndex, UINT, MinVertexIndex, UINT, NumVertices, UINT, startIndex, UINT, primCount)
WRAP(DrawPrimitiveUP, D3DPRIMITIVETYPE, primitive_type,UINT, primitive_count, const void *,data, UINT, stride)
WRAP(DrawIndexedPrimitiveUP, D3DPRIMITIVETYPE, primitive_type, UINT, min_vertex_idx, UINT, vertex_count,UINT, primitive_count, const void *,index_data, D3DFORMAT, index_format, const void *,data, UINT, stride)
WRAP(ProcessVertices, UINT, SrcStartIndex, UINT, DestIndex, UINT, VertexCount, IDirect3DVertexBuffer9*, pDestBuffer, IDirect3DVertexDeclaration9*, pVertexDecl, DWORD, Flags)
WRAP(CreateVertexDeclaration, const D3DVERTEXELEMENT9 *,elements,IDirect3DVertexDeclaration9 **,declaration)
WRAP(SetVertexDeclaration, IDirect3DVertexDeclaration9*, pDecl)
WRAP(GetVertexDeclaration, IDirect3DVertexDeclaration9**, ppDecl)
WRAP(SetFVF, DWORD, FVF)
WRAP(GetFVF, DWORD*, pFVF)
WRAP(CreateVertexShader, const DWORD*,byte_code, IDirect3DVertexShader9 **,shader)
WRAP(SetVertexShader, IDirect3DVertexShader9*, pShader)
WRAP(GetVertexShader, IDirect3DVertexShader9**, ppShader)
WRAP(SetVertexShaderConstantF, UINT, reg_idx, const float *,data, UINT, count)
WRAP(GetVertexShaderConstantF, UINT, StartRegister, float*, pConstantData, UINT, Vector4fCount)
WRAP(SetVertexShaderConstantI, UINT, reg_idx, const int *,data, UINT, count)
WRAP(GetVertexShaderConstantI, UINT, StartRegister, int*, pConstantData, UINT, Vector4iCount)
WRAP(SetVertexShaderConstantB, UINT, reg_idx, const WINBOOL*,data, UINT, count)
WRAP(GetVertexShaderConstantB, UINT, StartRegister, WINBOOL*, pConstantData, UINT, BoolCount)
WRAP(SetStreamSource, UINT, StreamNumber, IDirect3DVertexBuffer9*, pStreamData, UINT, OffsetInBytes, UINT, Stride)
WRAP(GetStreamSource, UINT, StreamNumber, IDirect3DVertexBuffer9**, ppStreamData, UINT*, OffsetInBytes, UINT*, pStride)
WRAP(SetStreamSourceFreq, UINT, StreamNumber, UINT, Divider)
WRAP(GetStreamSourceFreq, UINT, StreamNumber, UINT*, Divider)
WRAP(SetIndices, IDirect3DIndexBuffer9*, pIndexData)
WRAP(GetIndices, IDirect3DIndexBuffer9**, ppIndexData)
WRAP(CreatePixelShader, const DWORD*,byte_code, IDirect3DPixelShader9 **,shader)
WRAP(SetPixelShader, IDirect3DPixelShader9*, pShader)
WRAP(GetPixelShader, IDirect3DPixelShader9**, ppShader)
WRAP(SetPixelShaderConstantF, UINT, reg_idx, const float *,data, UINT, count)
WRAP(GetPixelShaderConstantF, UINT, StartRegister, float*, pConstantData, UINT, Vector4fCount)
WRAP(SetPixelShaderConstantI, UINT, reg_idx, const int *,data, UINT, count)
WRAP(GetPixelShaderConstantI, UINT, StartRegister, int*, pConstantData, UINT, Vector4iCount)
WRAP(SetPixelShaderConstantB, UINT, reg_idx, const WINBOOL*,data, UINT, count)
WRAP(GetPixelShaderConstantB, UINT, StartRegister, WINBOOL*, pConstantData, UINT, BoolCount)
WRAP(DrawRectPatch, UINT, handle, const float *,segment_count, const D3DRECTPATCH_INFO *,patch_info)
WRAP(DrawTriPatch, UINT, handle, const float *,segment_count, const D3DTRIPATCH_INFO *,patch_info)
WRAP(DeletePatch, UINT, Handle)
WRAP(CreateQuery, D3DQUERYTYPE, Type, IDirect3DQuery9**, ppQuery)
#undef THIS__
#undef TYPE__
#undef PREFIX__

#define THIS__ buffer
#define TYPE__ IDirect3DVertexBuffer9*
#define PREFIX__ D3D9_VertexBuffer
WRAP(Lock, UINT, OffsetToLock, UINT, SizeToLock, void**, ppbData, DWORD, Flags)
WRAP(Unlock)
WRAP(GetDesc, D3DVERTEXBUFFER_DESC*, pDesc)
#undef THIS__
#undef TYPE__
#undef PREFIX__

static inline void* D3D_GetBufferPointer(D3DBlob buffer)
{
   return buffer->lpVtbl->GetBufferPointer(buffer);
}

static inline size_t D3D_GetBufferSize(D3DBlob buffer)
{
   return buffer->lpVtbl->GetBufferSize(buffer);
}
