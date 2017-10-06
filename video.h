
#include <X11/Xutil.h>


typedef struct
{
   Display* display;
   Window window;
}video_t;

extern video_t video;

void video_init();
void video_frame();
void video_destroy();
