
#include <X11/Xutil.h>

#include "display.h"
#include "platform.h"
#include "video.h"

platform_t platform;


void platform_init()
{
   platform.running = true;
}

void platform_destroy()
{
}

void platform_handle_events()
{
   XEvent e;
   if(XCheckWindowEvent(display.display, display.window, ~0, &e) && (e.type == KeyPress))
   {
      if(e.xkey.keycode == XKeysymToKeycode(display.display, XK_q))
         platform.running = false;
   }


}
