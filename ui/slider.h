#pragma once

#include "hitbox.h"
#include "vulkan/slider.h"

typedef struct
{
   hitbox_t hitbox[3];
   int x;
   int y;
   int width;
   int height;
   float pos;
   float size;
   bool grab;
}slider_t;

void slider_init(slider_t* slider);
void slider_destroy(slider_t* slider);
void slider_draw(slider_t* slider);
