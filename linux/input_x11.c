
#include <string.h>
#include <X11/Xutil.h>

#include "input.h"
#include "video.h"
#include "common.h"
#include "interface.h"

extern Atom wmDeleteMessage;

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

   while (XPending(video.screens[0].display))
   {
      XNextEvent(video.screens[0].display, &e);

      switch (e.type)
      {
      case KeyPress:
      case KeyRelease:
         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_q))
            input.pad.buttons.A = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_w))
            input.pad.buttons.B = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_a))
            input.pad.buttons.X = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_s))
            input.pad.buttons.Y = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_Up))
            input.pad.buttons.up = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_Down))
            input.pad.buttons.down = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_Left))
            input.pad.buttons.left = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_Right))
            input.pad.buttons.right = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_e))
            input.pad.buttons.L = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_d))
            input.pad.buttons.R = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_Return))
            input.pad.buttons.start = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_Shift_R))
            input.pad.buttons.select = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_Escape))
            input.pad.meta.exit = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_F4))
            input.pad.meta.menu = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_F5))
            input.pad.meta.vsync = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_F6))
            input.pad.meta.filter = (e.type == KeyPress);

         if (e.xkey.keycode == XKeysymToKeycode(video.screens[0].display, XK_F7))
            input.pad.meta.console = (e.type == KeyPress);

         break;

      case ButtonPress:
         if (e.xbutton.button == Button1)
            input.pointer.touch1 = 1;

         if (e.xbutton.button == Button2)
            input.pointer.touch2 = 1;

         if (e.xbutton.button == Button3)
            input.pointer.touch3 = 1;

         break;

      case ButtonRelease:
         if (e.xbutton.button == Button1)
            input.pointer.touch1 = 0;

         if (e.xbutton.button == Button2)
            input.pointer.touch2 = 0;

         if (e.xbutton.button == Button3)
            input.pointer.touch3 = 0;

         break;

      case MotionNotify:
         input.pointer.x = e.xmotion.x;
         input.pointer.y = e.xmotion.y;
         break;

      case DestroyNotify:
         input.pad.meta.exit = true;
//         video.screens[0].window = 0;
//         video.screens[0].display = NULL;
         printf("DestroyNotify\n");
         fflush(stdout);
         break;

//         case ConfigureNotify:
//            video.screens[i].width = e.xconfigure.width;
//            video.screens[i].height = e.xconfigure.height;
//            break;
      case ClientMessage:
         if (e.xclient.data.l[0] == wmDeleteMessage)
         {
            if (e.xclient.window == video.screens[0].window)
               input.pad.meta.exit = true;
         }

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
