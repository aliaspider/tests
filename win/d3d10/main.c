#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include <d3d10.h>

#include "common.h"
#include "video.h"
#include "input.h"

static void video_init()
{
   DEBUG_LINE();
}


static void video_render()
{

}

static void video_destroy()
{
   DEBUG_LINE();
}

static void video_register_draw_command(int screen_id, draw_command_t fn)
{
}


const video_t video_d3d10 =
{
   .init                  = video_init,
   .render                = video_render,
   .destroy               = video_destroy,
   .register_draw_command = video_register_draw_command,
};

