#pragma once

typedef struct hitbox_t hitbox_t;
struct hitbox_t
{
   int x;
   int y;
   int width;
   int height;
   void (*callback)(void* data);
   void* data;
#ifdef HITBOX_INTERNAL
   struct hitbox_t* next;
	struct hitbox_t* prev;
#else
   const void* private[2];
#endif
};

void hitbox_add(hitbox_t * hitbox);
void hitbox_remove(hitbox_t *hitbox);
void hitbox_check(void);
