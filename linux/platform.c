
#include <X11/Xutil.h>

#include "platform.h"
#include "video.h"

platform_t platform;

Atom wmDeleteMessage;

void platform_init()
{
#ifdef HAVE_X11
   XInitThreads();
   Display *display = XOpenDisplay(NULL);
   wmDeleteMessage = XInternAtom(display, "WM_DELETE_WINDOW", False);
   int i;

   for (i = 0; i < video.screen_count; i++)
   {
      video.screens[i].display = display;
      video.screens[i].window  = XCreateSimpleWindow(video.screens[i].display,
            DefaultRootWindow(video.screens[i].display), 0, 0, video.screens[i].width, video.screens[i].height, 0, 0, 0);
      XStoreName(video.screens[i].display, video.screens[i].window, "Vulkan Test");
      XSelectInput(video.screens[i].display, video.screens[i].window,
         ExposureMask | FocusChangeMask | KeyPressMask | KeyReleaseMask | ButtonPressMask | ButtonReleaseMask | PointerMotionMask);
      XSetWMProtocols(video.screens[i].display, video.screens[i].window, &wmDeleteMessage, 1);
      XMapWindow(video.screens[i].display, video.screens[i].window);
   }

#endif
}

void platform_destroy()
{
#ifdef HAVE_X11
   Display *display = video.screens[0].display;

   int i;

   for (i = 0; i < video.screen_count; i++)
   {
      XDestroyWindow(video.screens[i].display, video.screens[i].window);
      video.screens[i].display = NULL;
   }

   XCloseDisplay(display);
#endif
}

