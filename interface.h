#ifndef INTERFACE_H__
#define INTERFACE_H__

#include <stdint.h>
#include <stdbool.h>
#include <stdio.h>

#ifdef __cplusplus
extern "C"
{
#endif

void console_log(const char* fmt, ...);
#define printf       console_log
#define debug_log    console_log

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

typedef enum
{
   PAD_BUTTON_A,
   PAD_BUTTON_B,
   PAD_BUTTON_X,
   PAD_BUTTON_Y,
   PAD_BUTTON_UP,
   PAD_BUTTON_DOWN,
   PAD_BUTTON_LEFT,
   PAD_BUTTON_RIGHT,
   PAD_BUTTON_L,
   PAD_BUTTON_R,
   PAD_BUTTON_START,
   PAD_BUTTON_SELECT,
   PAD_BUTTON_MAX,
}buttons_name_t;
typedef union
{
   struct
   {
      int A :1;
      int B :1;
      int X :1;
      int Y :1;

      int up :1;
      int down :1;
      int left :1;
      int right :1;

      int L :1;
      int R :1;
      int start :1;
      int select :1;
   };
   uint32_t mask;
}buttons_t;

typedef union
{
   struct
   {
      int exit :1;
      int menu :1;
      int vsync :1;
   };
   uint32_t mask;
}meta_buttons_t;

typedef struct
{
   union
   {
      struct
      {
         buttons_t buttons;
         meta_buttons_t meta;
      };
      uint64_t mask;
   };

   struct
   {
      float x;
      float y;
   }left_stick;

   struct
   {
      float x;
      float y;
   }right_stick;

}pad_t;

typedef struct
{
   int x;
   int y;
   bool touch1;
   bool touch2;
   bool touch3;
}pointer_t;

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
   pad_t* pad;
} module_run_info_t;

void module_init(const module_init_info_t *init_info, module_info_t *module_info);
void module_destroy();
void module_run(module_run_info_t *run_info);

extern module_info_t module;

#ifdef __cplusplus
}
#endif

#endif // INTERFACE_H__
