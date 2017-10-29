#include <dsound.h>
#include <unistd.h>

#include "audio.h"
#include "common.h"
#include "video.h"
#include "interface.h"

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

