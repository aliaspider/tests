#define HITBOX_INTERNAL

#include <stddef.h>

#include "hitbox.h"
#include "input.h"

static hitbox_t *first;

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
}

void hitbox_check(void)
{
	hitbox_t *hitbox = first;

	while(hitbox)
	{
		if(input.pointer.x >= hitbox->x && input.pointer.x < hitbox->x + hitbox->width &&
         input.pointer.y >= hitbox->y && input.pointer.y < hitbox->y + hitbox->height)
      {
			hitbox->callback(hitbox->data);
         return;
      }

		hitbox = hitbox->next;
	}
}
