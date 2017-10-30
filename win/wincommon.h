#pragma once

typedef int8_t s8;
typedef uint8_t u8;
typedef int16_t s16;
typedef uint16_t u16;
typedef __LONG32 s32;
typedef unsigned __LONG32 u32;
typedef int64_t s64;
typedef uint64_t u64;

//char err_str[256];
//FormatMessage(FORMAT_MESSAGE_FROM_SYSTEM, NULL, HRESULT_CODE(hr), 0, err_str, sizeof(err_str), NULL);
//printf("%s\n", err_str);

#define CHECK_WINERR(x) do{HRESULT hr = x; if(FAILED(hr)) {\
   printf("error at %s:%i:%s: (%i, %i, %i) 0x%08X(%i)\n", \
   __FILE__, __LINE__, __FUNCTION__, HRESULT_SEVERITY(hr), HRESULT_FACILITY(hr), HRESULT_CODE(hr), hr, hr);\
   fflush(stdout);assert(0);}}while(0)

#define DEBUG_WINERR(x) do{printf("(0x%X, 0x%X, 0x%X) 0x%08X\n", HRESULT_SEVERITY(hr), HRESULT_FACILITY(hr), HRESULT_CODE(hr), hr);fflush(stdout);}while(0)

#if defined(__DINPUT_INCLUDED__) || defined(DIRECTINPUT_HEADER_VERSION) || defined(__DSOUND_INCLUDED__) || defined(DS_OK)

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

#define CONCAT_(a,b) a##b
#define CONCAT(a,b) CONCAT_(a,b)

//#define WRAP(type,fn,...) static inline type CONCAT(PREFIX__,fn) (MERGE_TYPE(__VA_ARGS__)) {return DROP_TYPE(THIS_)->lpVtbl->fn(DROP_TYPE(__VA_ARGS__));}
#define WRAP_(type,fn,...) static inline void CONCAT(PREFIX__,fn) (TYPE__ THIS__ MERGE_TYPE(__VA_ARGS__)) {CHECK_WINERR(THIS__->lpVtbl->fn(THIS__ DROP_TYPE( __VA_ARGS__)));}
#define WRAP(...) WRAP_(HRESULT, __VA_ARGS__)

#define WRAPNOCHECK_(type,fn,...) static inline type CONCAT(PREFIX__,fn) (TYPE__ THIS__ MERGE_TYPE(__VA_ARGS__)) {return THIS__->lpVtbl->fn(THIS__ DROP_TYPE( __VA_ARGS__));}
#define WRAPNOCHECK(...) WRAPNOCHECK_(HRESULT, __VA_ARGS__)

#define IUNKNOWN__ \
   WRAP_(HRESULT, QueryInterface,  REFIID, riid, void**, Object) \
   WRAP_(ULONG, AddRef) \
   WRAP_(ULONG, Release) \

#endif

#if defined(__DINPUT_INCLUDED__) || defined(DIRECTINPUT_HEADER_VERSION)
#include <dinput.h>
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

#ifdef UNICODE
#define LPDIENUMEFFECTSCALLBACK LPDIENUMEFFECTSCALLBACKW
#else
#define LPDIENUMEFFECTSCALLBACK LPDIENUMEFFECTSCALLBACKA
#endif

#define THIS__ device
#define TYPE__ IDirectInputDevice8*
#define PREFIX__ DIDEV8_
IUNKNOWN__
WRAP(GetCapabilities, LPDIDEVCAPS, DIDevCaps)
WRAP(EnumObjects, LPDIENUMDEVICEOBJECTSCALLBACK, Callback, void*, Ref, u32, Flags)
WRAP(GetProperty, REFGUID, rguidProp, LPDIPROPHEADER, pdiph)
WRAP(SetProperty, REFGUID, rguidProp, LPCDIPROPHEADER, pdiph)
WRAPNOCHECK(Acquire)
WRAP(Unacquire)
WRAPNOCHECK(GetDeviceState, u32, Size, void*, Data)
WRAP(GetDeviceData, u32, Size, DIDEVICEOBJECTDATA*, DevObjData, u32*, InOut, u32, Flags)
WRAP(SetDataFormat, LPCDIDATAFORMAT, format)
WRAP(SetEventNotification, HANDLE, Event)
WRAP(SetCooperativeLevel, HWND, hwnd, u32, Flags)
WRAP(GetObjectInfo, DIDEVICEOBJECTINSTANCE*, DevObjInstance, u32, Obj, u32, How)
WRAP(GetDeviceInfo, DIDEVICEINSTANCE*, DevInstance)
WRAP(RunControlPanel, HWND, hwndOwner, u32, Flags)
WRAP(Initialize, HINSTANCE, hinst, u32, Version, REFGUID, guid)
WRAP(CreateEffect, REFGUID, guid, LPCDIEFFECT, eff, LPDIRECTINPUTEFFECT*, ppdeff, IUnknown*, unkOuter)
WRAP(EnumEffects, LPDIENUMEFFECTSCALLBACK, Callback, void*, Ref, u32, EffType)
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
#endif

#if defined(__DSOUND_INCLUDED__) || defined(DS_OK)
#include <dsound.h>
#define THIS__ dsound8
#define TYPE__ IDirectSound8*
#define PREFIX__ DS8_
IUNKNOWN__
WRAP(CreateSoundBuffer, const DSBUFFERDESC*, DSBufferDesc, IDirectSoundBuffer**, DirectSoundBuffer, IUnknown*, UnkOuter)
WRAP(GetCaps, LPDSCAPS, DSCaps)
WRAP(DuplicateSoundBuffer, IDirectSoundBuffer*, DsbOriginal, IDirectSoundBuffer**, DsbDuplicate)
WRAP(SetCooperativeLevel, HWND, hwnd, u32, Level)
WRAP(Compact)
WRAP(GetSpeakerConfig, u32*, SpeakerConfig)
WRAP(SetSpeakerConfig, u32, SpeakerConfig)
WRAP(Initialize, LPCGUID, Guid)
WRAP(VerifyCertification, u32*, Certified)
#undef THIS__
#undef TYPE__
#undef PREFIX__

#define THIS__ dsbuffer8
#define TYPE__ IDirectSoundBuffer8*
#define PREFIX__ DSb8_
IUNKNOWN__
WRAP(GetCaps, DSBCAPS*, DSBufferCaps)
WRAP(GetCurrentPosition, u32*, CurrentPlayCursor, u32*, CurrentWriteCursor)
WRAP(GetFormat, WAVEFORMATEX*, wfxFormat, u32, SizeAllocated, u32*, SizeWritten)
WRAP(GetVolume, s32*, Volume)
WRAP(GetPan, s32*, an)
WRAP(GetFrequency, u32*, Frequency)
WRAP(GetStatus, u32*, Status)
WRAP(Initialize, IDirectSound*, DirectSound, const DSBUFFERDESC*, DSBufferDesc)
WRAP(Lock, u32, Offset, u32, Bytes, void**, AudioPtr1, u32*, AudioBytes1, void**, AudioPtr2, u32*, AudioBytes2,
     u32, Flags)
WRAP(Play, u32, Reserved1, u32, Reserved2, u32, Flags)
WRAP(SetCurrentPosition, u32, NewPosition)
WRAP(SetFormat, const WAVEFORMATEX*, fxFormat)
WRAP(SetVolume, s32, Volume)
WRAP(SetPan, s32, Pan)
WRAP(SetFrequency, u32, Frequency)
WRAP(Stop)
WRAP(Unlock, void*, AudioPtr1, u32, AudioBytes1, void*, AudioPtr2, u32, AudioBytes2)
WRAP(Restore)
WRAP(SetFX, u32, EffectsCount, DSEFFECTDESC*, DSFXDesc, u32*, ResultCodes)
WRAP(AcquireResources, u32, Flags, u32, EffectsCount, u32*, ResultCodes)
WRAP(GetObjectInPath, REFGUID, Object, u32, Index, REFGUID, Interface, void**, ppObject)
#undef THIS__
#undef TYPE__
#undef PREFIX__

#define THIS__ dsbuffer
#define TYPE__ IDirectSoundBuffer*
#define PREFIX__ DSb_
IUNKNOWN__
WRAP(GetCaps, DSBCAPS*, DSBufferCaps)
WRAP(GetCurrentPosition, u32*, CurrentPlayCursor, u32*, CurrentWriteCursor)
WRAP(GetFormat, WAVEFORMATEX*, wfxFormat, u32, SizeAllocated, u32*, SizeWritten)
WRAP(GetVolume, s32*, Volume)
WRAP(GetPan, s32*, an)
WRAP(GetFrequency, u32*, Frequency)
WRAP(GetStatus, u32*, Status)
WRAP(Initialize, IDirectSound*, DirectSound, const DSBUFFERDESC*, DSBufferDesc)
WRAP(Lock, u32, Offset, u32, Bytes, void**, AudioPtr1, u32*, AudioBytes1, void**, AudioPtr2, u32*, AudioBytes2, u32,
     Flags)
WRAP(Play, u32, Reserved1, u32, Priority, u32, Flags)
WRAP(SetCurrentPosition, u32, NewPosition)
WRAP(SetFormat, const WAVEFORMATEX*, fxFormat)
WRAP(SetVolume, s32, Volume)
WRAP(SetPan, s32, Pan)
WRAP(SetFrequency, u32, Frequency)
WRAP(Stop)
WRAP(Unlock, void*, AudioPtr1, u32, AudioBytes1, void*, AudioPtr2, u32, AudioBytes2)
WRAP(Restore)
#undef THIS__
#undef TYPE__
#undef PREFIX__
#endif
