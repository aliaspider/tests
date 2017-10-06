
#ifdef HAVE_X11
#include <X11/Xutil.h>
#endif

typedef struct
{
   int width;
   int height;
#ifdef HAVE_X11
   Display *display;
   Window   window;
#endif
} display_t;

extern display_t display;

void display_init(int width, int height);
void display_destroy();
