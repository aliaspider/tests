#include "vulkan_common.h"


typedef struct
{
   vec4 pos;
   vec4 coords;
} sprite_t;

extern vk_renderer_t sprite_renderer;

void vk_sprite_add(sprite_t* sprite, vk_texture_t* texture);
