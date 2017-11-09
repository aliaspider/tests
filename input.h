#pragma once

#include "interface.h"

typedef struct
{
   void (*init)();
   void (*destroy)();
   void (*update)();
   pad_t pad;
   pad_t pad_pressed;
   pad_t pad_released;
   pointer_t pointer;
}input_t;

#ifdef __WIN32__
extern const input_t input_dinput;
#endif
#ifdef HAVE_X11
extern const input_t input_x11;
#endif
#ifdef _3DS
extern const input_t input_3ds;
#endif
extern input_t input;


