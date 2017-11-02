#pragma once

#include "hitbox.h"

#ifdef BUTTON_INTERNAL
#define const
#endif

typedef struct button_t button_t;
struct button_t
{
   hitbox_t hitbox;
   const bool clicked;
};

#ifdef BUTTON_INTERNAL
#undef const
#endif

void button_init(button_t * button);
void button_destroy(button_t *button);
void button_update(button_t * button);
