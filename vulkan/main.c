#define _GNU_SOURCE
#include <stdio.h>
#include <unistd.h>
#include <string.h>
#include <assert.h>
#include <stdarg.h>

#include "common.h"
#include "video.h"
#include "input.h"
#include "vulkan_common.h"
#include "frame.h"
#include "font.h"
#include "slider.h"
#include "sprite.h"

static vk_context_t vk;
static vk_render_target_t RTarget[MAX_SCREENS];
static VkCommandBuffer cmd;

static vk_renderer_t *renderers[] =
{
   &R_frame,
   &R_sprite,
   &R_font,
   &R_slider,
   NULL
};


void console_draw(screen_t *screen);
void fps_draw(screen_t *screen)
{
   font_render_options_t options =
   {
      .max_width = screen->width,
      .max_height = screen->height,
   };
   vk_font_draw_text(video.fps, &options);
}

void screen_id_draw(screen_t *screen)
{
   char buffer[16];
   snprintf(buffer, sizeof(buffer), "SCREEN: \e[%im%i", RED, (int)(screen - video.screens));

   font_render_options_t options =
   {
      .x = screen->width - 20 - 9 * 12,
      .y = screen->width < 400 ? screen->height - 22 : 0,
      .max_width = screen->width,
      .max_height = screen->height,
   };
   vk_font_draw_text(buffer, &options);
}

void frame_draw(screen_t *screen)
{
   static bool is_menu = false;

   if (input.pad_pressed.meta.menu)
      is_menu = !is_menu;

   if (is_menu)
   {

   }
   else
      vk_frame_add(0, 0, screen->width, screen->height);
}

void frame_draw_small(screen_t *screen)
{
   vk_frame_add(screen->width - 256 - 20, 22, 256, 224);
}

vk_texture_t test_image;

void sprite_test(screen_t *screen)
{
   static int calls = 0;
   test_image.staging.mem.u8[calls++] = 0xFF;

   if (calls > test_image.staging.mem.size / 2)
      calls = 0;

   test_image.flushed = false;
   test_image.uploaded = false;
   test_image.dirty = true;
   {
      sprite_t sprite =
      {
         .pos.values = {330.0, 100.0, (calls >> 3) & 0xFF, 256.0},
         .coords.values = {0.0, 0.0, 256.0, 256.0},
         .color.values = {0.4, 1.0, 0.5, 0.20},
      };
      vk_sprite_add(&sprite, &test_image);
   }
   {
      sprite_t sprite =
      {
         .pos.values = {100.0, 40.0, 128, 32.0},
         .color.values = {1.0, 0.0, 0.5, 0.50},
      };
      vk_sprite_add(&sprite, NULL);
   }
   {
      sprite_t sprite =
      {
         .pos.values = {320.0, 400.0, 320.0, 64.0},
         .coords.values = {20.0, 20.0, 132.0, 32.0},
         .color.values = {0.0, 1.0, 1.0, 0.50},
      };
      vk_sprite_add(&sprite, &R_frame.tex);
   }
   {
      sprite_t sprite =
      {
         .pos.values = {10.0, 190.0, R_font.tex.width, R_font.tex.height},
         .coords.values = {0.0, 0.0, R_font.tex.width, R_font.tex.height},
         .color.values = {1.0, 1.0, 0.0, 1.0},
      };
      vk_sprite_add(&sprite, &R_font.tex);
   }
}

typedef struct
{
   char *msg;
   int frames;
   unsigned screen_mask;
   font_render_options_t options;
} msg_buffer_t;
msg_buffer_t msg_buffer[64];

static void display_message_handler(screen_t *screen)
{
   msg_buffer_t *ptr = msg_buffer;

   while (ptr->msg && ptr - msg_buffer < countof(msg_buffer))
   {
      if (ptr->screen_mask & (1 << (screen - video.screens)))
      {
         ptr->options.max_width = screen->width;
         ptr->options.max_height = screen->height;
         vk_font_draw_text(ptr->msg, &ptr->options);
         ptr->frames--;

         if (!ptr->frames)
         {
            free(ptr->msg);
            ptr->msg = NULL;

            if (msg_buffer + countof(msg_buffer) - 1 - ptr > 0)
               memmove(ptr, ptr + 1, (msg_buffer + countof(msg_buffer) - 1 - ptr) * sizeof(msg_buffer_t));

            continue;
         }
      }

      ptr++;
   }
}

void display_message(int frames, int x, int y, unsigned screen_mask, const char *fmt, ...)
{
   msg_buffer_t *ptr = msg_buffer;

   while (ptr->msg && ptr - msg_buffer < countof(msg_buffer))
      ptr++;

   if (ptr - msg_buffer == countof(msg_buffer))
      return;

   va_list va;
   va_start(va, fmt);
   vasprintf(&ptr->msg, fmt, va);
   va_end(va);
   ptr->frames = frames;
   ptr->screen_mask = screen_mask;
   ptr->options.x = x;
   ptr->options.y = y;
}

void video_init()
{
   debug_log("video init\n");
   debug_log("color test : \e[%im%s \e[%im%s \e[%im%s \e[%im%s \e[%im%s \e[%im%s "
             "\e[%im%s \e[%im%s \e[%im%s \e[%im%s \e[%im%s \e[%im%s \e[%im%s \e[%im%s \e[%im%s \e[%im%s \e[39m\n",
             BLACK, "BLACK", RED, "RED", GREEN, "GREEN", YELLOW, "YELLOW",
             BLUE, "BLUE", MAGENTA, "MAGENTA", CYAN, "CYAN", LIGHT_GRAY, "LIGHT_GRAY",
             DARK_GRAY, "DARK_GRAY", LIGHT_RED, "LIGHT_RED", LIGHT_GREEN, "LIGHT_GREEN", LIGHT_YELLOW, "LIGHT_YELLOW",
             LIGHT_BLUE, "LIGHT_BLUE", LIGHT_MAGENTA, "LIGHT_MAGENTA", LIGHT_CYAN, "LIGHT_CYAN", WHITE, "WHITE");

   vk_context_init(&vk);

   VkAllocateCommandBuffer(vk.device, vk.pools.cmd, &cmd);

   vk_render_targets_init(&vk, video.screen_count, video.screens, RTarget);

   for (vk_renderer_t **renderer = renderers; *renderer; renderer++)
      (*renderer)->init(&vk);

   vk_register_draw_command(&RTarget[0].draw_list, frame_draw);
   vk_register_draw_command(&RTarget[0].draw_list, sprite_test);
   vk_register_draw_command(&RTarget[0].draw_list, fps_draw);
   vk_register_draw_command(&RTarget[0].draw_list, screen_id_draw);
   vk_register_draw_command(&RTarget[0].draw_list, display_message_handler);

   vk_register_draw_command(&RTarget[1].draw_list, frame_draw_small);
   vk_register_draw_command(&RTarget[1].draw_list, sprite_test);
   vk_register_draw_command(&RTarget[1].draw_list, fps_draw);
   vk_register_draw_command(&RTarget[1].draw_list, screen_id_draw);
   vk_register_draw_command(&RTarget[1].draw_list, console_draw);
   vk_register_draw_command(&RTarget[1].draw_list, display_message_handler);

//   vk_register_draw_command(&render_targets[2].draw_list, frame_draw);
//   vk_register_draw_command(&render_targets[2].draw_list, fps_draw);
//   vk_register_draw_command(&render_targets[2].draw_list, screen_id_draw);

   test_image.width = 256;
   test_image.height = 256;
   test_image.format = VK_FORMAT_R5G6B5_UNORM_PACK16;
   vk_texture_init(&vk, &test_image);
   memset(test_image.staging.mem.u8 + test_image.staging.mem.layout.offset, 0x80,
          test_image.staging.mem.size - test_image.staging.mem.layout.offset);
}

void video_render()
{
   uint32_t image_indices[MAX_SCREENS];
   VkSwapchainKHR swapchains[MAX_SCREENS];

   R_frame.tex.dirty = true;

//   vkWaitForFences(vk.device, 1, &vk.queue_fence, VK_TRUE, UINT64_MAX);
   VK_CHECK(vkWaitForFences(vk.device, 1, &vk.queue_fence, VK_TRUE, 100000000));
   vkResetFences(vk.device, 1, &vk.queue_fence);

   VkBeginCommandBuffer(cmd, VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT, NULL);

   for (int i = 0; i < video.screen_count; i++)
   {
      vkWaitForFences(vk.device, 1, &RTarget[i].chain_fence, VK_TRUE, UINT64_MAX);
      vkResetFences(vk.device, 1, &RTarget[i].chain_fence);

      if (R_frame.tex.filter != video.filter)
      {
         R_frame.tex.filter = video.filter ? VK_FILTER_LINEAR : VK_FILTER_NEAREST;
         R_frame.tex.info.sampler = video.filter ? vk.samplers.linear : vk.samplers.nearest;
         vk_texture_update_descriptor_sets(&vk, &R_frame.tex);
      }

      if (video.vsync != RTarget[i].vsync)
      {
         vk_swapchain_destroy(&vk, &RTarget[i]);
         vk_swapchain_init(&vk, &RTarget[i]);
      }

      while (vkAcquireNextImageKHR(vk.device, RTarget[i].swapchain, UINT64_MAX, NULL,
                                   RTarget[i].chain_fence, &image_indices[i]) != VK_SUCCESS)
      {
         usleep(100000);
         vk_swapchain_destroy(&vk, &RTarget[i]);
         vk_swapchain_init(&vk, &RTarget[i]);
      }

//      vkWaitForFences(vk.device, 1, &display_fence, VK_TRUE, UINT64_MAX);
//      vkResetFences(vk.device, 1, &display_fence);


      for (vk_drawcmd_list_t *draw_cmd = RTarget[i].draw_list; draw_cmd; draw_cmd = draw_cmd->next)
         draw_cmd->draw(RTarget[i].screen);

      for (vk_renderer_t **renderer = renderers; *renderer; renderer++)
         (*renderer)->update(vk.device, cmd, *renderer);

      /* renderpass */
      {
         const VkClearValue clearValue = {{{0.0f, 0.1f, 1.0f, 0.0f}}};
         float pc[2] = {RTarget[i].viewport.width, RTarget[i].viewport.height};
         VkCmdBeginRenderPass(cmd, vk.renderpass, RTarget[i].framebuffers[image_indices[i]],
                              RTarget[i].scissor, &clearValue);
         vkCmdPushConstants(cmd, vk.pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(pc), pc);

         vkCmdSetViewport(cmd, 0, 1, &RTarget[i].viewport);
         vkCmdSetScissor(cmd, 0, 1, &RTarget[i].scissor);

         for (vk_renderer_t **renderer = renderers; *renderer; renderer++)
            (*renderer)->exec(cmd, *renderer);

         vkCmdEndRenderPass(cmd);
      }

      swapchains[i] = RTarget[i].swapchain;
   }

   vkEndCommandBuffer(cmd);

   for (vk_renderer_t **renderer = renderers; *renderer; renderer++)
      (*renderer)->finish(vk.device, *renderer);

   VkQueueSubmit(vk.queue, 1, &cmd, vk.queue_fence);
   VkQueuePresent(vk.queue, video.screen_count, swapchains, image_indices);


#if 0
   {
      uint64_t vblank_counter = 0;
      VK_CHECK(vkGetSwapchainCounterEXT(vk.device, chain.handle, VK_SURFACE_COUNTER_VBLANK_EXT,
                                        &vblank_counter));
      debug_log("vblank_counter : %" PRId64 "\n", vblank_counter);
   }
#endif
}

void video_destroy()
{
   int i;

   for (i = 0; i < video.screen_count; i++)
      vkWaitForFences(vk.device, 1, &RTarget[i].chain_fence, VK_TRUE, UINT64_MAX);

   vkWaitForFences(vk.device, 1, &vk.queue_fence, VK_TRUE, UINT64_MAX);

   for (vk_renderer_t **renderer = renderers; *renderer; renderer++)
      (*renderer)->destroy(vk.device, *renderer);

   vk_texture_free(vk.device, &test_image);
   vk_render_targets_destroy(&vk, video.screen_count, RTarget);
   vk_context_destroy(&vk);

   video.frame.data = NULL;
   debug_log("video destroy\n");
}

void video_register_draw_command(int screen_id, draw_command_t fn)
{
   vk_register_draw_command(&RTarget[screen_id].draw_list, fn);
}

const video_t video_vulkan =
{
   .init = video_init,
   .render = video_render,
   .destroy = video_destroy,
   .register_draw_command = video_register_draw_command,
};
