#pragma once

#include <stdlib.h>

#ifdef HAVE_X11
#include <X11/Xutil.h>
#endif
#ifdef WIN32
#include <windows.h>
#endif
#include "interface.h"

#define MAX_SCREENS 4
typedef struct
{
   int id;
   int x;
   int y;
   int width;
   int height;
#ifdef HAVE_X11
   Display* display;
   Window window;
#endif
#ifdef WIN32
   HINSTANCE hinstance;
   HWND hwnd;
#endif
}screen_t;

typedef struct
{
   int width;
   int height;
   int pitch;
   void* data;
}frame_t;

typedef void(*draw_command_t)(screen_t *screen);
typedef struct vk_drawcmd_list_t
{
   draw_command_t draw;
   struct vk_drawcmd_list_t *next;
} vk_drawcmd_list_t;

static inline void vk_register_draw_command(vk_drawcmd_list_t **list, draw_command_t fn)
{
   while (*list)
      list = &(*list)->next;

   *list = malloc(sizeof(**list));

   (*list)->draw = fn;
   (*list)->next = NULL;
}

static inline void vk_remove_draw_command(vk_drawcmd_list_t **list, draw_command_t fn)
{
   /* TODO */
   vk_drawcmd_list_t **prev = NULL;

   while (*list)
   {
      if ((*list)->draw == fn)
      {
         vk_drawcmd_list_t *tmp = (*list)->next;
         free(*list);

         if (prev)
            (*prev)->next = tmp;

         *list = tmp;

         return;
      }

      prev = list;
      list = &(*list)->next;
   }
}

typedef struct video_t
{
   void (*init)();
   void (*render)();
   void (*destroy)();
   void (*register_draw_command)(int screen_id, draw_command_t fn);
   int screen_count;
   screen_t screens[MAX_SCREENS];
   frame_t frame;
   char fps[256];
   bool vsync;
   bool filter;
}video_t;


extern const video_t video_vulkan;
extern const video_t video_d3d12;
extern video_t video;

