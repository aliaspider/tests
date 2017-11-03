#define SLIDER_INTERNAL

#include <string.h>

#include "slider.h"
#include "common.h"
#include "input.h"

static void slider_update_hitbox(slider_t *slider)
{
   float pos = slider->pos > 0.0 ? slider->pos < 1.0 ? slider->pos : 1.0 : 0.0;
   slider->hitbox[0].x = slider->x;
   slider->hitbox[0].y = slider->y;
   slider->hitbox[0].width = slider->width;
   slider->hitbox[0].height = slider->height * pos * (1.0 - slider->size);

   slider->hitbox[1].x = slider->x;
   slider->hitbox[1].y = slider->hitbox[0].y + slider->hitbox[0].height;
   slider->hitbox[1].width = slider->width;
   slider->hitbox[1].height = slider->height * slider->size;

   slider->hitbox[2].x = slider->x;
   slider->hitbox[2].y = slider->hitbox[1].y + slider->hitbox[1].height;
   slider->hitbox[2].width = slider->width;
   slider->hitbox[2].height = slider->height - slider->hitbox[0].height - slider->hitbox[1].height;

}

static void slider_scroll(slider_t *slider)
{
   if (input.pointer.touch1_pressed)
   {
      slider->hitbox[0].grab = false;
      slider->hitbox[1].grab = true;
      slider->hitbox[2].grab = false;
      slider->pos = (((input.pointer.y - slider->y) / (float)(slider->height)) / (1.0 - slider->size)) -
                    (slider->size / 2.0) / (1.0 - slider->size);
      slider->pos = slider->pos > 0.0 ? slider->pos < 1.0 ? slider->pos : 1.0 : 0.0;
      slider_update_hitbox(slider);
   }
}

static void slider_grab(slider_t *slider)
{
   if (slider->hitbox[1].grab)
      slider->pos += input.pointer.dy / (float)(slider->height * (1.0 - slider->size));
   else
      slider->pos = slider->pos > 0.0 ? slider->pos < 1.0 ? slider->pos : 1.0 : 0.0;
}

void slider_init(slider_t *slider)
{
   slider->hitbox[0].data = slider;
   slider->hitbox[1].data = slider;
   slider->hitbox[2].data = slider;

   slider->hitbox[0].callback = (void *)slider_scroll;
   slider->hitbox[1].callback = (void *)slider_grab;
   slider->hitbox[2].callback = (void *)slider_scroll;

   slider_update_hitbox(slider);

   hitbox_add(&slider->hitbox[0]);
   hitbox_add(&slider->hitbox[1]);
   hitbox_add(&slider->hitbox[2]);
}

void slider_destroy(slider_t *slider)
{
   hitbox_remove(&slider->hitbox[0]);
   hitbox_remove(&slider->hitbox[1]);
   hitbox_remove(&slider->hitbox[2]);
   memset(slider, 0x00, sizeof(*slider));
}

void slider_update(slider_t *slider)
{
   slider->start = (1.0 - slider->size) * (slider->pos > 0.0 ? slider->pos < 1.0 ? slider->pos : 1.0 : 0.0);

   slider_update_hitbox(slider);
   vk_slider_add(slider->x, slider->y, slider->width, slider->height, slider->start, slider->size);
}
