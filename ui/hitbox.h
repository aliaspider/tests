#pragma once

typedef struct
{
   int x;
   int y;
   int width;
   int height;
   void (*callback)(void* data);
   void* data;
   const void* private[2];
}hitbox_t;

void hitbox_add(hitbox_t * hitbox);
void hitbox_remove(hitbox_t *hitbox);
void hitbox_check(void);
