
#include <string.h>
#include <X11/Xutil.h>

#include "input.h"
#include "video.h"
#include "common.h"
#include "interface.h"

void input_x11_init()
{
   memset(&input.pad, 0, sizeof(input.pad));
   XAutoRepeatOn(video.screens[0].display);
}

void input_x11_destroy()
{
   XAutoRepeatOn(video.screens[0].display);
}

void input_x11_update()
{
   XEvent e;

   int i;
   for (i = 0; i < video.screen_count; i++)
   {
      while (XCheckWindowEvent(video.screens[0].display, video.screens[i].window, ~0, &e))
      {
         switch (e.type)
         {
         case KeyPress:
         case KeyRelease:
            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_q))
               input.pad.buttons.A = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_w))
               input.pad.buttons.B = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_a))
               input.pad.buttons.X = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_s))
               input.pad.buttons.Y = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_Up))
               input.pad.buttons.up = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_Down))
               input.pad.buttons.down = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_Left))
               input.pad.buttons.left = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_Right))
               input.pad.buttons.right = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_e))
               input.pad.buttons.L = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_d))
               input.pad.buttons.R = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_Return))
               input.pad.buttons.start = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_Shift_R))
               input.pad.buttons.select = (e.type == KeyPress);

            if (e.xkey.keycode == XKeysymToKeycode(video.screens[i].display, XK_Escape))
               input.pad.meta.exit = (e.type == KeyPress);

            break;

         case ButtonPress:
            if(e.xbutton.button == Button1)
               input.pointer.touch1 = 1;
            break;

         case ButtonRelease:
            if(e.xbutton.button == Button1)
               input.pointer.touch1 = 0;
            break;

         case MotionNotify:
            input.pointer.x = e.xmotion.x;
            input.pointer.y = e.xmotion.y;
            break;

//         case FocusIn:
//            XAutoRepeatOff(video.screens[i].display);
//            break;

//         case FocusOut:
//            XAutoRepeatOn(video.screens[i].display);
//            break;
         }

      }

   }
}

const input_t input_x11 =
{
   .init = input_x11_init,
   .destroy = input_x11_destroy,
   .update = input_x11_update
};
