#define BUTTON_INTERNAL

#include <string.h>

#include "button.h"
#include "common.h"
#include "input.h"
#include "vulkan/sprite.h"

//static void button_hit(button_t *button)
//{

//}

void button_init(button_t *button)
{
   button->hitbox.data = button;

//   button->hitbox.callback = (void *)button_hit;

   hitbox_add(&button->hitbox);
}

void button_destroy(button_t *button)
{
   hitbox_remove(&button->hitbox);
   memset(button, 0x00, sizeof(*button));
}

void button_update(button_t *button)
{
   sprite_t sprite = {};
   sprite.pos.x = button->x;
   sprite.pos.y = button->y;
   sprite.pos.width = button->width;
   sprite.pos.height = button->height;
   sprite.effect.edge = true;
   sprite.effect.gloss = true;
   sprite.color = button->clicked? 0xFFFFFFFF :button->hitbox.hit? 0xFF0000FF : 0x80404040;
   vk_sprite_add(&sprite, NULL);
}
