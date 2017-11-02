
#include <stddef.h>

#include "hitbox.h"
#include "input.h"

typedef struct hitbox_internal_t
{
	int x;
	int y;
	int width;
	int height;
	void (*callback)(void* data);
	void* data;
	struct hitbox_internal_t* next;
	struct hitbox_internal_t* prev;
}hitbox_internal_t;

_Static_assert(sizeof (hitbox_t) == sizeof(hitbox_internal_t), "sizeof (hitbox_t) != sizeof(hitbox_internal_t)");

static hitbox_internal_t *first;

void hitbox_add(hitbox_t * hitbox_)
{
	hitbox_internal_t * hitbox = (hitbox_internal_t *)hitbox_;

	hitbox_internal_t **last = &first;
	hitbox_internal_t * prev = NULL;

	while (*last)
	{
		prev = *last;
		last = &(*last)->next;
	}

	*last = hitbox;

	hitbox->prev = prev;
	hitbox->next = NULL;
}

void hitbox_remove(hitbox_t *hitbox_)
{
	hitbox_internal_t *hitbox = (hitbox_internal_t *)hitbox_;

	if (hitbox->next)
		hitbox->next->prev = hitbox->prev;

	if (hitbox->prev)
		hitbox->prev->next = hitbox->next;
	else
		first = hitbox->next;
}

void hitbox_check(void)
{
	hitbox_internal_t *hitbox = first;

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
