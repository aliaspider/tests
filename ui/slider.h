#pragma once

#include "hitbox.h"
#include "vulkan/slider.h"

#ifdef SLIDER_INTERNAL
#define const
#endif

typedef struct
{
   hitbox_t hitbox[3];
   int x;
   int y;
   int width;
   int height;
   float pos;
   float size;
   const float real_pos;
   const bool grab;
}slider_t;

#ifdef SLIDER_INTERNAL
#undef const
#endif

void slider_init(slider_t* slider);
void slider_destroy(slider_t* slider);
void slider_update(slider_t* slider);

