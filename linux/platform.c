
#include <X11/Xutil.h>

#include "platform.h"
#include "video.h"

platform_t platform;


void platform_init()
{
#ifdef HAVE_X11
   XInitThreads();
#endif

   platform.running = true;
}

void platform_destroy()
{
}

void platform_handle_events()
{
   XEvent e;
   if(XCheckWindowEvent(video.screen.display, video.screen.window, ~0, &e) && (e.type == KeyPress))
   {
      if(e.xkey.keycode == XKeysymToKeycode(video.screen.display, XK_q))
         platform.running = false;
   }


}
