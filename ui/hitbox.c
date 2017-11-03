#define HITBOX_INTERNAL

#include <stddef.h>

#include "hitbox.h"
#include "input.h"

static hitbox_t *first;
static hitbox_t *last_hit;

void hitbox_add(hitbox_t * hitbox)
{
	hitbox_t **last = &first;
	hitbox_t * prev = NULL;

	while (*last)
	{
		prev = *last;
		last = &(*last)->next;
	}

	*last = hitbox;

	hitbox->prev = prev;
	hitbox->next = NULL;
}

void hitbox_remove(hitbox_t *hitbox)
{
	if (hitbox->next)
		hitbox->next->prev = hitbox->prev;

	if (hitbox->prev)
		hitbox->prev->next = hitbox->next;
	else
		first = hitbox->next;

   if(last_hit == hitbox)
      last_hit = NULL;
}

void hitbox_check(void)
{
   if(last_hit && last_hit->grab)
   {
      if(!input.pointer.touch1)
         last_hit->grab = false;
      else
      {
         if(last_hit->callback)
            last_hit->callback(last_hit->data);
         return;
      }
   }

	hitbox_t *hitbox = first;

	while(hitbox)
	{
		if(input.pointer.x >= hitbox->x && input.pointer.x < hitbox->x + hitbox->width &&
         input.pointer.y >= hitbox->y && input.pointer.y < hitbox->y + hitbox->height)
      {
         if(last_hit != hitbox)
         {
            if(last_hit)
            {
               last_hit->hit = false;
//               last_hit->callback(last_hit->data);
            }
            last_hit = hitbox;
         }

         hitbox->hit = true;
         if(input.pointer.touch1_pressed)
            hitbox->grab = true;

         if(hitbox->callback)
            hitbox->callback(hitbox->data);
         return;
      }

		hitbox = hitbox->next;
	}

   if(last_hit)
   {
      last_hit->hit = false;
//      last_hit->callback(last_hit->data);
      last_hit = NULL;
   }

}
