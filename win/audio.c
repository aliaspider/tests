#include <dsound.h>
#include <unistd.h>

#include "audio.h"
#include "common.h"
#include "video.h"
#include "interface.h"

#define WRAP(type,fn,...) static inline type CONCAT(PREFIX__,fn) (MERGE_TYPE(__VA_ARGS__)) {return DROP_TYPE(THIS_)->lpVtbl->fn(DROP_TYPE(__VA_ARGS__));}
#undef THIS_

#define THIS_ LPDIRECTSOUND8, dSound8
#define PREFIX__ DS_
WRAP(HRESULT, CreateSoundBuffer, THIS_, LPCDSBUFFERDESC, lpcDSBufferDesc, LPLPDIRECTSOUNDBUFFER, lplpDirectSoundBuffer,
     IUnknown*, pUnkOuter)
WRAP(HRESULT, GetCaps, THIS_, LPDSCAPS, lpDSCaps)
WRAP(HRESULT, DuplicateSoundBuffer, THIS_, LPDIRECTSOUNDBUFFER, lpDsbOriginal, LPLPDIRECTSOUNDBUFFER, lplpDsbDuplicate)
WRAP(HRESULT, SetCooperativeLevel, THIS_, HWND, hwnd, DWORD, dwLevel)
WRAP(HRESULT, Compact, THIS_)
WRAP(HRESULT, GetSpeakerConfig, THIS_, LPDWORD, lpdwSpeakerConfig)
WRAP(HRESULT, SetSpeakerConfig, THIS_, DWORD, dwSpeakerConfig)
WRAP(HRESULT, Initialize, THIS_, LPCGUID, lpcGuid)
WRAP(HRESULT, VerifyCertification, THIS_, LPDWORD, pdwCertified)
#undef THIS_
#undef PREFIX__

#define THIS_ LPDIRECTSOUNDBUFFER8, dSoundBuffer8
#define PREFIX__ DSB_
WRAP(HRESULT, GetCaps, THIS_, LPDSBCAPS, lpDSBufferCaps)
WRAP(HRESULT, GetCurrentPosition, THIS_, LPDWORD, lpdwCurrentPlayCursor, LPDWORD, lpdwCurrentWriteCursor)
WRAP(HRESULT, GetFormat, THIS_, LPWAVEFORMATEX, lpwfxFormat, DWORD, dwSizeAllocated, LPDWORD, lpdwSizeWritten)
WRAP(HRESULT, GetVolume, THIS_, LPLONG, lplVolume)
WRAP(HRESULT, GetPan, THIS_, LPLONG, lplpan)
WRAP(HRESULT, GetFrequency, THIS_, LPDWORD, lpdwFrequency)
WRAP(HRESULT, GetStatus, THIS_, LPDWORD, lpdwStatus)
WRAP(HRESULT, Initialize, THIS_, LPDIRECTSOUND, lpDirectSound, LPCDSBUFFERDESC, lpcDSBufferDesc)
WRAP(HRESULT, Lock, THIS_, DWORD, dwOffset, DWORD, dwBytes, LPVOID*, ppvAudioPtr1, LPDWORD, pdwAudioBytes1, LPVOID*,
     ppvAudioPtr2, LPDWORD, pdwAudioBytes2, DWORD, dwFlags)
WRAP(HRESULT, Play, THIS_, DWORD, dwReserved1, DWORD, dwReserved2, DWORD, dwFlags)
WRAP(HRESULT, SetCurrentPosition, THIS_, DWORD, dwNewPosition)
WRAP(HRESULT, SetFormat, THIS_, LPCWAVEFORMATEX, lpcfxFormat)
WRAP(HRESULT, SetVolume, THIS_, LONG, lVolume)
WRAP(HRESULT, SetPan, THIS_, LONG, lPan)
WRAP(HRESULT, SetFrequency, THIS_, DWORD, dwFrequency)
WRAP(HRESULT, Stop, THIS_)
WRAP(HRESULT, Unlock, THIS_, LPVOID, pvAudioPtr1, DWORD, dwAudioBytes1, LPVOID, pvAudioPtr2, DWORD, dwAudioPtr2)
WRAP(HRESULT, Restore, THIS_)
WRAP(HRESULT, SetFX, THIS_, DWORD, dwEffectsCount, LPDSEFFECTDESC, pDSFXDesc, LPDWORD, pdwResultCodes)
WRAP(HRESULT, AcquireResources, THIS_, DWORD, dwFlags, DWORD, dwEffectsCount, LPDWORD, pdwResultCodes)
WRAP(HRESULT, GetObjectInPath, THIS_, REFGUID, rguidObject, DWORD, dwIndex, REFGUID, rguidInterface, LPVOID*, ppObject)
#undef THIS_
#undef PREFIX__

#define THIS_ LPDIRECTSOUNDBUFFER, dSoundBuffer
#define PREFIX__ dsb_
WRAP(HRESULT, QueryInterface, THIS_,  REFIID, riid, void**, ppvObject)
WRAP(ULONG, AddRef, THIS_)
WRAP(ULONG, Release, THIS_)
WRAP(HRESULT, GetCaps, THIS_, LPDSBCAPS, lpDSBufferCaps)
WRAP(HRESULT, GetCurrentPosition, THIS_, LPDWORD, lpdwCurrentPlayCursor, LPDWORD, lpdwCurrentWriteCursor)
WRAP(HRESULT, GetFormat, THIS_, LPWAVEFORMATEX, lpwfxFormat, DWORD, dwSizeAllocated, LPDWORD, lpdwSizeWritten)
WRAP(HRESULT, GetVolume, THIS_, LPLONG, lplVolume)
WRAP(HRESULT, GetPan, THIS_, LPLONG, lplpan)
WRAP(HRESULT, GetFrequency, THIS_, LPDWORD, lpdwFrequency)
WRAP(HRESULT, GetStatus, THIS_, LPDWORD, lpdwStatus)
WRAP(HRESULT, Initialize, THIS_, LPDIRECTSOUND, lpDirectSound, LPCDSBUFFERDESC, lpcDSBufferDesc)
WRAP(HRESULT, Lock, THIS_, DWORD, dwOffset, DWORD, dwBytes, LPVOID*, ppvAudioPtr1, LPDWORD, pdwAudioBytes1, LPVOID*,
     ppvAudioPtr2, LPDWORD, pdwAudioBytes2, DWORD, dwFlags)
WRAP(HRESULT, Play, THIS_, DWORD, dwReserved1, DWORD, dwPriority, DWORD, dwFlags)
WRAP(HRESULT, SetCurrentPosition, THIS_, DWORD, dwNewPosition)
WRAP(HRESULT, SetFormat, THIS_, LPCWAVEFORMATEX, lpcfxFormat)
WRAP(HRESULT, SetVolume, THIS_, LONG, lVolume)
WRAP(HRESULT, SetPan, THIS_, LONG, lPan)
WRAP(HRESULT, SetFrequency, THIS_, DWORD, dwFrequency)
WRAP(HRESULT, Stop, THIS_)
WRAP(HRESULT, Unlock, THIS_, LPVOID, pvAudioPtr1, DWORD, dwAudioBytes1, LPVOID, pvAudioPtr2, DWORD, dwAudioPtr2)
WRAP(HRESULT, Restore, THIS_)
#undef THIS_
#undef PREFIX__

static LPDIRECTSOUND8 dsound;
static LPDIRECTSOUNDBUFFER8 ds_buffer;

#define DS_BUFFER_SIZE (1 << 14)

static uint8_t* buffer_start;
static uint8_t* write_ptr;

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
      .dwBufferBytes = DS_BUFFER_SIZE,
      .lpwfxFormat = &wfx,
   };

   LPDIRECTSOUNDBUFFER pDsb;

   CHECK_WINERR(DS_CreateSoundBuffer(dsound, &dsbdesc, &pDsb, NULL));
//   CHECK_WINERR(dsb_QueryInterface(pDsb, &IID_IDirectSound8, (void**) &ds_buffer));
   CHECK_WINERR((dsb_QueryInterface(pDsb, &IID_IDirectSoundBuffer8, (void**) &ds_buffer)));
   CHECK_WINERR(dsb_Release(pDsb));

   void* ptr1;
   DWORD size1;
   void* ptr2;
   DWORD size2;
   DSB_Lock(ds_buffer, 0, DS_BUFFER_SIZE, &ptr1, &size1, &ptr2, &size2, DSBLOCK_ENTIREBUFFER);

   memset(ptr1, 0x00, size1);

   if (size2)
      memset(ptr2, 0x00, size2);

   DSB_Unlock(ds_buffer, ptr1 , size1, ptr2, size2);

   DSB_Play(ds_buffer, 0, 0, DSBPLAY_LOOPING);

   write_ptr = buffer_start = ptr1;

}
void audio_destroy(void)
{

}
void audio_play(void* buffer, int samples)
{
   void* ptr1 = NULL;
   DWORD size1;
   void* ptr2 = NULL;
   DWORD size2;



//   while(((write_ptr - buffer_start - write_cursor ) & (DS_BUFFER_SIZE - 1)) > 1000)
//   {
//      usleep(1000);
//      printf("sleep\n");
//      DSB_GetCurrentPosition(ds_buffer, &play_cursor, &write_cursor);
//   }

   while(true)
   {
      DWORD play_cursor;
      DWORD write_cursor;
      DSB_GetCurrentPosition(ds_buffer, &play_cursor, &write_cursor);
      if((((write_ptr - buffer_start - write_cursor ) & (DS_BUFFER_SIZE - 1)) < 2048))
         break;
      usleep(100);
//      printf("sleep\n");
   }

   CHECK_WINERR(DSB_Lock(ds_buffer, (write_ptr - buffer_start) & (DS_BUFFER_SIZE - 1), 4 * samples, &ptr1, &size1, &ptr2, &size2, 0));


   memcpy(ptr1, buffer, size1);
   if(ptr2)
   {
      memcpy(ptr2, (uint8_t*)buffer + size1, size2);
      write_ptr = (uint8_t*)ptr2 + size2;
   }
   else
      write_ptr = (uint8_t*)ptr1 + size1;

   DSB_Unlock(ds_buffer, ptr1 , size1, ptr2, size2);


}

const audio_t audio_win =
{
   .init = audio_init,
   .destroy = audio_destroy,
   .play = audio_play
};

