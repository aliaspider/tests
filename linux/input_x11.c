
#include <string.h>
#include <X11/Xutil.h>

#include "input.h"
#include "video.h"
#include "common.h"
#include "interface.h"

void input_x11_init()
{
   memset(&input.pad, 0, sizeof(input.pad));
}

void input_x11_destroy()
{
   XAutoRepeatOn(video.screen.display);
}

void input_x11_update()
{
   XEvent e;


   while (XCheckWindowEvent(video.screen.display, video.screen.window, ~0, &e))
   {
      switch (e.type)
      {
      case KeyPress:
      case KeyRelease:
         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_q))
            input.pad.buttons.A = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_w))
            input.pad.buttons.B = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_a))
            input.pad.buttons.X = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_s))
            input.pad.buttons.Y = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_Up))
            input.pad.buttons.up = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_Down))
            input.pad.buttons.down = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_Left))
            input.pad.buttons.left = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_Right))
            input.pad.buttons.right = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_e))
            input.pad.buttons.L = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_d))
            input.pad.buttons.R = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_Return))
            input.pad.buttons.start = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_Shift_R))
            input.pad.buttons.select = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_Escape))
            input.pad.meta.exit = (e.type == KeyPress);

         break;

      case FocusIn:
         XAutoRepeatOff(video.screen.display);
         break;

      case FocusOut:
         XAutoRepeatOn(video.screen.display);
         break;
      }

   }

}

const input_t input_x11 =
{
   .init = input_x11_init,
   .destroy = input_x11_destroy,
   .update = input_x11_update
};
