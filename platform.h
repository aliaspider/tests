#pragma once

#include <stdbool.h>

#ifdef HAVE_X11
#include <X11/Xutil.h>
#endif

#ifdef __WIN32__
#include <windows.h>
#endif

typedef struct
{
#ifdef __WIN32__
   HINSTANCE hInstance;
#endif
} platform_t;

extern platform_t platform;

void platform_init();
void platform_destroy();
void platform_update();
