
#include <stdbool.h>

#ifdef HAVE_X11
#include <X11/Xutil.h>
#endif

typedef struct
{
} platform_t;

extern platform_t platform;

void platform_init();
void platform_destroy();
