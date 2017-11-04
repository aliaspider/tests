#include "vulkan_common.h"


typedef struct
{
   vec4 pos;
   vec4 coords;
   uint32_t color;
   struct
   {
      uint8_t edge;
      uint8_t gloss;
   }effect;
} sprite_t;

extern vk_renderer_t R_sprite;

void vk_sprite_add(sprite_t* sprite, vk_texture_t* texture);
