
#include <X11/Xutil.h>

#include "platform.h"
#include "video.h"

platform_t platform;


void platform_init()
{
#ifdef HAVE_X11
   XInitThreads();
   video.screen.display = XOpenDisplay(NULL);
   video.screen.window  = XCreateSimpleWindow(video.screen.display,
         DefaultRootWindow(video.screen.display), 0, 0, video.screen.width, video.screen.height, 0, 0, 0);
   XStoreName(video.screen.display, video.screen.window, "Vulkan Test");
   XSelectInput(video.screen.display, video.screen.window,
      ExposureMask | FocusChangeMask | KeyPressMask | KeyReleaseMask);
   XMapWindow(video.screen.display, video.screen.window);
#endif
}

void platform_destroy()
{   
#ifdef HAVE_X11
   XDestroyWindow(video.screen.display, video.screen.window);
   XCloseDisplay(video.screen.display);
   video.screen.display = NULL;
#endif
}

