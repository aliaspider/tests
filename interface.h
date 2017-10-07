#ifndef INTERFACE_H__
#define INTERFACE_H__

#include <stdint.h>
#include <stdbool.h>

#ifdef __cplusplus
extern "C"
{
#endif

#define debug_log printf

typedef struct
{
   const char *filename;
} module_init_info_t;

typedef enum
{
   screen_format_ARGB8888,
   screen_format_RGB565,
   screen_format_ARGB5551,
} screen_format_t;

typedef struct
{
   int output_width;
   int output_height;
   screen_format_t screen_format;
   bool stereo;
   float framerate;
   float audio_rate;
} module_info_t;

typedef union
{
   void* ptr;
   uint8_t* u8;
   uint16_t* u16;
   uint32_t* u32;
   int8_t* s8;
   int16_t* s16;
   int32_t* s32;
}generic_ptr_t;

typedef struct
{
   generic_ptr_t screen;
   int pitch;
   generic_ptr_t sound_buffer;
   unsigned max_samples;
   bool frame_completed;
} module_run_info_t;

void module_init(const module_init_info_t *init_info, module_info_t *module_info);
void module_destroy();
void module_run(module_run_info_t *run_info);

extern const char* test_file;

#ifdef __cplusplus
}
#endif

#endif // INTERFACE_H__
