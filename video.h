
#include <X11/Xutil.h>


typedef struct
{
   int width;
   int height;
#ifdef HAVE_X11
   Display* display;
   Window window;
#endif
}screen_t;

typedef struct video_t
{
   void (*init)();
   void (*frame)();
   void (*destroy)();
   screen_t screen;
}video_t;

extern const video_t video_vulkan;
extern video_t video;

