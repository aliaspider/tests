
#include <stdbool.h>
#include <X11/Xutil.h>

typedef struct
{
} platform_t;

extern platform_t platform;

void platform_init();
void platform_destroy();
