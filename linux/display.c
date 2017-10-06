
#include "display.h"

display_t display;

void display_init(int width, int height)
{
   display.width = width;
   display.height = height;
   XInitThreads();
   display.display = XOpenDisplay(NULL);
   display.window  = XCreateSimpleWindow(display.display, DefaultRootWindow(display.display), 0, 0, display.width, display.height, 0, 0, 0);
   XStoreName(display.display, display.window, "Vulkan Test");
   XSelectInput(display.display, display.window, ExposureMask | KeyPressMask);
   XMapWindow(display.display, display.window);

}

void display_destroy()
{
   XDestroyWindow(display.display, display.window);
   XCloseDisplay(display.display);
   display.display = NULL;

}
