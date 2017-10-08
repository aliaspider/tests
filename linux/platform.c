
#include <X11/Xutil.h>

#include "platform.h"
#include "video.h"

platform_t platform;


void platform_init()
{
#ifdef HAVE_X11
   XInitThreads();
#endif
}

void platform_destroy()
{   
}

