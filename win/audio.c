#include <dsound.h>

#include "audio.h"
#include "common.h"
#include "video.h"
#include "interface.h"

#define WRAP(type,fn,...) static inline type CONCAT(PREFIX__,fn) (MERGE_TYPE(__VA_ARGS__)) {return DROP_TYPE(THIS_)->lpVtbl->fn(DROP_TYPE(__VA_ARGS__));}
#undef THIS_

#define THIS_ LPDIRECTSOUND8, dSound8
#define PREFIX__ DS_
WRAP(HRESULT,CreateSoundBuffer, THIS_, LPCDSBUFFERDESC, lpcDSBufferDesc, LPLPDIRECTSOUNDBUFFER, lplpDirectSoundBuffer, IUnknown*, pUnkOuter)
WRAP(HRESULT,GetCaps, THIS_, LPDSCAPS, lpDSCaps)
WRAP(HRESULT,DuplicateSoundBuffer, THIS_, LPDIRECTSOUNDBUFFER, lpDsbOriginal, LPLPDIRECTSOUNDBUFFER, lplpDsbDuplicate)
WRAP(HRESULT,SetCooperativeLevel, THIS_, HWND, hwnd, DWORD, dwLevel)
WRAP(HRESULT,Compact, THIS_)
WRAP(HRESULT,GetSpeakerConfig, THIS_, LPDWORD, lpdwSpeakerConfig)
WRAP(HRESULT,SetSpeakerConfig, THIS_, DWORD, dwSpeakerConfig)
WRAP(HRESULT,Initialize, THIS_, LPCGUID, lpcGuid)
WRAP(HRESULT,VerifyCertification, THIS_, LPDWORD, pdwCertified)
#undef THIS_
#undef PREFIX__

#define THIS_ LPDIRECTSOUNDBUFFER8, dSoundBuffer8
#define PREFIX__ DSB_
WRAP(HRESULT,GetCaps, THIS_, LPDSBCAPS, lpDSBufferCaps)
WRAP(HRESULT,GetCurrentPosition, THIS_, LPDWORD, lpdwCurrentPlayCursor, LPDWORD, lpdwCurrentWriteCursor)
WRAP(HRESULT,GetFormat, THIS_, LPWAVEFORMATEX, lpwfxFormat, DWORD, dwSizeAllocated, LPDWORD, lpdwSizeWritten)
WRAP(HRESULT,GetVolume, THIS_, LPLONG, lplVolume)
WRAP(HRESULT,GetPan, THIS_, LPLONG, lplpan)
WRAP(HRESULT,GetFrequency, THIS_, LPDWORD, lpdwFrequency)
WRAP(HRESULT,GetStatus, THIS_, LPDWORD, lpdwStatus)
WRAP(HRESULT,Initialize, THIS_, LPDIRECTSOUND, lpDirectSound, LPCDSBUFFERDESC, lpcDSBufferDesc)
WRAP(HRESULT,Lock, THIS_, DWORD, dwOffset, DWORD, dwBytes, LPVOID*, ppvAudioPtr1, LPDWORD, pdwAudioBytes1, LPVOID*, ppvAudioPtr2, LPDWORD, pdwAudioBytes2, DWORD, dwFlags)
WRAP(HRESULT,Play, THIS_, DWORD, dwReserved1, DWORD, dwReserved2, DWORD, dwFlags)
WRAP(HRESULT,SetCurrentPosition, THIS_, DWORD, dwNewPosition)
WRAP(HRESULT,SetFormat, THIS_, LPCWAVEFORMATEX, lpcfxFormat)
WRAP(HRESULT,SetVolume, THIS_, LONG, lVolume)
WRAP(HRESULT,SetPan, THIS_, LONG, lPan)
WRAP(HRESULT,SetFrequency, THIS_, DWORD, dwFrequency)
WRAP(HRESULT,Stop, THIS_)
WRAP(HRESULT,Unlock, THIS_, LPVOID, pvAudioPtr1, DWORD, dwAudioBytes1, LPVOID, pvAudioPtr2, DWORD, dwAudioPtr2)
WRAP(HRESULT,Restore, THIS_)
WRAP(HRESULT,SetFX, THIS_, DWORD, dwEffectsCount, LPDSEFFECTDESC, pDSFXDesc, LPDWORD, pdwResultCodes)
WRAP(HRESULT,AcquireResources, THIS_, DWORD, dwFlags, DWORD, dwEffectsCount, LPDWORD, pdwResultCodes)
WRAP(HRESULT,GetObjectInPath, THIS_, REFGUID, rguidObject, DWORD, dwIndex, REFGUID, rguidInterface, LPVOID*, ppObject)
#undef THIS_
#undef PREFIX__

#define THIS_ LPDIRECTSOUNDBUFFER, dSoundBuffer
#define PREFIX__ dsb_
WRAP(HRESULT,QueryInterface, THIS_,  REFIID, riid, void**, ppvObject)
WRAP(ULONG,AddRef,THIS_)
WRAP(ULONG,Release,THIS_)
WRAP(HRESULT,GetCaps, THIS_, LPDSBCAPS, lpDSBufferCaps)
WRAP(HRESULT,GetCurrentPosition, THIS_, LPDWORD, lpdwCurrentPlayCursor, LPDWORD, lpdwCurrentWriteCursor)
WRAP(HRESULT,GetFormat, THIS_, LPWAVEFORMATEX, lpwfxFormat, DWORD, dwSizeAllocated, LPDWORD, lpdwSizeWritten)
WRAP(HRESULT,GetVolume, THIS_, LPLONG, lplVolume)
WRAP(HRESULT,GetPan, THIS_, LPLONG, lplpan)
WRAP(HRESULT,GetFrequency, THIS_, LPDWORD, lpdwFrequency)
WRAP(HRESULT,GetStatus, THIS_, LPDWORD, lpdwStatus)
WRAP(HRESULT,Initialize, THIS_, LPDIRECTSOUND, lpDirectSound, LPCDSBUFFERDESC, lpcDSBufferDesc)
WRAP(HRESULT,Lock, THIS_, DWORD, dwOffset, DWORD, dwBytes, LPVOID*, ppvAudioPtr1, LPDWORD, pdwAudioBytes1, LPVOID*, ppvAudioPtr2, LPDWORD, pdwAudioBytes2, DWORD, dwFlags)
WRAP(HRESULT,Play, THIS_, DWORD, dwReserved1, DWORD, dwReserved2, DWORD, dwFlags)
WRAP(HRESULT,SetCurrentPosition, THIS_, DWORD, dwNewPosition)
WRAP(HRESULT,SetFormat, THIS_, LPCWAVEFORMATEX, lpcfxFormat)
WRAP(HRESULT,SetVolume, THIS_, LONG, lVolume)
WRAP(HRESULT,SetPan, THIS_, LONG, lPan)
WRAP(HRESULT,SetFrequency, THIS_, DWORD, dwFrequency)
WRAP(HRESULT,Stop, THIS_)
WRAP(HRESULT,Unlock, THIS_, LPVOID, pvAudioPtr1, DWORD, dwAudioBytes1, LPVOID, pvAudioPtr2, DWORD, dwAudioPtr2)
WRAP(HRESULT,Restore, THIS_)
#undef THIS_
#undef PREFIX__

static LPDIRECTSOUND8 dsound;
static LPDIRECTSOUNDBUFFER8 ds_buffer;

void audio_init(void)
{
   CHECK_WINERR(DirectSoundCreate8(NULL, &dsound, NULL));
   CHECK_WINERR(DS_SetCooperativeLevel(dsound, video.screens[0].hwnd, DSSCL_PRIORITY));

   WAVEFORMATEX wfx =
   {
      .wFormatTag = WAVE_FORMAT_PCM,
      .nChannels = 2,
      .nSamplesPerSec = module.audio_rate,
      .nAvgBytesPerSec = 4 * module.audio_rate,
      .nBlockAlign = 4,
      .wBitsPerSample = 16,
   };

   DSBUFFERDESC dsbdesc =
   {
      .dwSize = sizeof(DSBUFFERDESC),
      .dwFlags = DSBCAPS_CTRLPAN | DSBCAPS_CTRLVOLUME | DSBCAPS_CTRLFREQUENCY | DSBCAPS_GLOBALFOCUS,
      .dwBufferBytes = 3 * wfx.nAvgBytesPerSec,
      .lpwfxFormat = &wfx,
   };

   LPDIRECTSOUNDBUFFER pDsb;

   CHECK_WINERR(DS_CreateSoundBuffer(dsound, &dsbdesc, &pDsb, NULL));
//   CHECK_WINERR(dsb_QueryInterface(pDsb, &IID_IDirectSound8, (void**) &ds_buffer));
   CHECK_WINERR((dsb_QueryInterface(pDsb, &IID_IDirectSoundBuffer8, (void**) &ds_buffer)));
   CHECK_WINERR(dsb_Release(pDsb));




}
void audio_destroy(void)
{

}
void audio_play(void* buffer, int samples)
{

}

const audio_t audio_win =
{
   .init = audio_init,
   .destroy = audio_destroy,
   .play = audio_play
};

