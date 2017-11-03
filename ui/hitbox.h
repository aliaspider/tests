#pragma once

#include <stdbool.h>

#ifdef HITBOX_INTERNAL
#define const
#endif

typedef struct hitbox_t hitbox_t;
struct hitbox_t
{
   int x;
   int y;
   int width;
   int height;
   const bool hit;
   union
   {
      bool grab;
      bool clicked;
   };
   void (*callback)(void* data);
   void* data;
#ifdef HITBOX_INTERNAL
   hitbox_t* next;
	hitbox_t* prev;
#else
   const void* private[2];
#endif
};

#ifdef HITBOX_INTERNAL
#undef const
#endif

typedef struct
{
   int x;
   int y;
   int dx;
   int dy;
   bool pressed;
   bool press;
   bool release;
   bool enter;
   bool leave;
}hit_info_t;

void hitbox_add(hitbox_t * hitbox);
void hitbox_remove(hitbox_t *hitbox);
void hitbox_check(void);
