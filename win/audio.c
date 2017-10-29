#include <dsound.h>
#include <unistd.h>

#include "audio.h"
#include "common.h"
#include "video.h"
#include "interface.h"

#undef THIS_

#define THIS_ LPDIRECTSOUND8, dSound8
#define PREFIX__ DS8_
WRAP(HRESULT, CreateSoundBuffer, THIS_, const DSBUFFERDESC*, lpcDSBufferDesc, IDirectSoundBuffer**,
     lplpDirectSoundBuffer,
     IUnknown*, pUnkOuter)
WRAP(HRESULT, GetCaps, THIS_, LPDSCAPS, lpDSCaps)
WRAP(HRESULT, DuplicateSoundBuffer, THIS_, IDirectSoundBuffer*, lpDsbOriginal, IDirectSoundBuffer**, lplpDsbDuplicate)
WRAP(HRESULT, SetCooperativeLevel, THIS_, HWND, hwnd, u32, dwLevel)
WRAP(HRESULT, Compact, THIS_)
WRAP(HRESULT, GetSpeakerConfig, THIS_, u32*, lpdwSpeakerConfig)
WRAP(HRESULT, SetSpeakerConfig, THIS_, u32, dwSpeakerConfig)
WRAP(HRESULT, Initialize, THIS_, LPCGUID, lpcGuid)
WRAP(HRESULT, VerifyCertification, THIS_, u32*, pdwCertified)
#undef THIS_
#undef PREFIX__

#define THIS_ IDirectSoundBuffer8*, dSoundBuffer8
#define PREFIX__ DSb8_
WRAP(HRESULT, GetCaps, THIS_, DSBCAPS*, lpDSBufferCaps)
WRAP(HRESULT, GetCurrentPosition, THIS_, u32*, lpdwCurrentPlayCursor, u32*, lpdwCurrentWriteCursor)
WRAP(HRESULT, GetFormat, THIS_, WAVEFORMATEX*, lpwfxFormat, u32, dwSizeAllocated, u32*, lpdwSizeWritten)
WRAP(HRESULT, GetVolume, THIS_, s32*, lplVolume)
WRAP(HRESULT, GetPan, THIS_, s32*, lplpan)
WRAP(HRESULT, GetFrequency, THIS_, u32*, lpdwFrequency)
WRAP(HRESULT, GetStatus, THIS_, u32*, lpdwStatus)
WRAP(HRESULT, Initialize, THIS_, IDirectSound*, lpDirectSound, const DSBUFFERDESC*, lpcDSBufferDesc)
WRAP(HRESULT, Lock, THIS_, u32, dwOffset, u32, dwBytes, void**, ppvAudioPtr1, u32*, pdwAudioBytes1, void**,
     ppvAudioPtr2, u32*, pdwAudioBytes2, u32, dwFlags)
WRAP(HRESULT, Play, THIS_, u32, dwReserved1, u32, dwReserved2, u32, dwFlags)
WRAP(HRESULT, SetCurrentPosition, THIS_, u32, dwNewPosition)
WRAP(HRESULT, SetFormat, THIS_, const WAVEFORMATEX*, lpcfxFormat)
WRAP(HRESULT, SetVolume, THIS_, s32, lVolume)
WRAP(HRESULT, SetPan, THIS_, s32, lPan)
WRAP(HRESULT, SetFrequency, THIS_, u32, dwFrequency)
WRAP(HRESULT, Stop, THIS_)
WRAP(HRESULT, Unlock, THIS_, void*, pvAudioPtr1, u32, dwAudioBytes1, void*, pvAudioPtr2, u32, dwAudioPtr2)
WRAP(HRESULT, Restore, THIS_)
WRAP(HRESULT, SetFX, THIS_, u32, dwEffectsCount, DSEFFECTDESC*, pDSFXDesc, u32*, pdwResultCodes)
WRAP(HRESULT, AcquireResources, THIS_, u32, dwFlags, u32, dwEffectsCount, u32*, pdwResultCodes)
WRAP(HRESULT, GetObjectInPath, THIS_, REFGUID, rguidObject, u32, dwIndex, REFGUID, rguidInterface, void**, ppObject)
#undef THIS_
#undef PREFIX__

#define THIS_ IDirectSoundBuffer*, dSoundBuffer
#define PREFIX__ DSb_
WRAP(HRESULT, QueryInterface, THIS_,  REFIID, riid, void**, ppvObject)
WRAP(ULONG, AddRef, THIS_)
WRAP(ULONG, Release, THIS_)
WRAP(HRESULT, GetCaps, THIS_, DSBCAPS*, lpDSBufferCaps)
WRAP(HRESULT, GetCurrentPosition, THIS_, u32*, lpdwCurrentPlayCursor, u32*, lpdwCurrentWriteCursor)
WRAP(HRESULT, GetFormat, THIS_, WAVEFORMATEX*, lpwfxFormat, u32, dwSizeAllocated, u32*, lpdwSizeWritten)
WRAP(HRESULT, GetVolume, THIS_, s32*, lplVolume)
WRAP(HRESULT, GetPan, THIS_, s32*, lplpan)
WRAP(HRESULT, GetFrequency, THIS_, u32*, lpdwFrequency)
WRAP(HRESULT, GetStatus, THIS_, u32*, lpdwStatus)
WRAP(HRESULT, Initialize, THIS_, IDirectSound*, lpDirectSound, const DSBUFFERDESC*, lpcDSBufferDesc)
WRAP(HRESULT, Lock, THIS_, u32, dwOffset, u32, dwBytes, void**, ppvAudioPtr1, u32*, pdwAudioBytes1, void**,
     ppvAudioPtr2, u32*, pdwAudioBytes2, u32, dwFlags)
WRAP(HRESULT, Play, THIS_, u32, dwReserved1, u32, dwPriority, u32, dwFlags)
WRAP(HRESULT, SetCurrentPosition, THIS_, u32, dwNewPosition)
WRAP(HRESULT, SetFormat, THIS_, const WAVEFORMATEX*, lpcfxFormat)
WRAP(HRESULT, SetVolume, THIS_, s32, lVolume)
WRAP(HRESULT, SetPan, THIS_, s32, lPan)
WRAP(HRESULT, SetFrequency, THIS_, u32, dwFrequency)
WRAP(HRESULT, Stop, THIS_)
WRAP(HRESULT, Unlock, THIS_, void*, pvAudioPtr1, u32, dwAudioBytes1, void*, pvAudioPtr2, u32, dwAudioPtr2)
WRAP(HRESULT, Restore, THIS_)
#undef THIS_
#undef PREFIX__

static IDirectSound8* dsound;
static IDirectSoundBuffer8* ds_buffer;

#define DS_BUFFER_SIZE (1 << 14)

static u8* buffer_start;
static u8* write_ptr;

void audio_init(void)
{
   CHECK_WINERR(DirectSoundCreate8(NULL, &dsound, NULL));
   CHECK_WINERR(DS8_SetCooperativeLevel(dsound, video.screens[0].hwnd, DSSCL_PRIORITY));

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

   IDirectSoundBuffer* pDsb;

   CHECK_WINERR(DS8_CreateSoundBuffer(dsound, &dsbdesc, &pDsb, NULL));
//   CHECK_WINERR(DSb_QueryInterface(pDsb, &IID_IDirectSound8, (void**) &ds_buffer));
   CHECK_WINERR((DSb_QueryInterface(pDsb, &IID_IDirectSoundBuffer8, (void**) &ds_buffer)));
   CHECK_WINERR(DSb_Release(pDsb));

   u32 size;
   DSb8_Lock(ds_buffer, 0, DS_BUFFER_SIZE, (void**)&buffer_start, &size, NULL, NULL, DSBLOCK_ENTIREBUFFER);

   memset(buffer_start, 0x00, size);


   DSb8_Unlock(ds_buffer, buffer_start, size, NULL, 0);

   DSb8_Play(ds_buffer, 0, 0, DSBPLAY_LOOPING);

   write_ptr = buffer_start;

}
void audio_destroy(void)
{

}
void audio_play(void* buffer, int samples)
{
   void* ptr1, *ptr2;
   u32 size1, size2;


   while (true)
   {
      u32 play_cursor;
      u32 write_cursor;
      DSb8_GetCurrentPosition(ds_buffer, &play_cursor, &write_cursor);

      if ((((write_ptr - buffer_start - write_cursor) & (DS_BUFFER_SIZE - 1)) < 2048 * 2))
         break;

      usleep(1);
//      printf("sleep\n");
   }

   CHECK_WINERR(DSb8_Lock(ds_buffer, (write_ptr - buffer_start) & (DS_BUFFER_SIZE - 1), 4 * samples, &ptr1, &size1, &ptr2,
                         &size2, 0));


   memcpy(ptr1, buffer, size1);

   if (ptr2)
   {
      memcpy(ptr2, (u8*)buffer + size1, size2);
      write_ptr = (u8*)ptr2 + size2;
   }
   else
      write_ptr = (u8*)ptr1 + size1;

   DSb8_Unlock(ds_buffer, ptr1 , size1, ptr2, size2);


}

const audio_t audio_win =
{
   .init = audio_init,
   .destroy = audio_destroy,
   .play = audio_play
};

