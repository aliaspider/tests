#pragma once

#include "hitbox.h"
#ifdef HAVE_VULKAN
#include "vulkan/common/texture.h"
#endif
#ifdef BUTTON_INTERNAL
#define const
#endif

typedef struct button_t button_t;
struct button_t
{
   union
   {
      struct
      {
         int x;
         int y;
         int width;
         int height;
         const bool clicked;
      };
      hitbox_t hitbox;
   };
#ifdef HAVE_VULKAN
   vk_texture_t* texture;
#endif
};

#ifdef BUTTON_INTERNAL
#undef const
#endif

void button_init(button_t * button);
void button_destroy(button_t *button);
void button_update(button_t * button);
