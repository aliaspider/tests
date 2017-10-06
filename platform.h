
#include <stdbool.h>
#include <X11/Xutil.h>

typedef struct
{
   bool running;
} platform_t;

extern platform_t platform;

void platform_init();
void platform_destroy();
void platform_handle_events();
