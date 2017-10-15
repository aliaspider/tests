

typedef struct
{
   void (*init)();
   void (*destroy)();
   void (*play)(void* buffer, int samples);
}audio_t;

extern const audio_t audio_win;
extern const audio_t audio_alsa;
extern audio_t audio;


