
#include <unistd.h>
#include <string.h>
#include <assert.h>

#include "common.h"
#include "video.h"
#include "input.h"
#include "vulkan_common.h"
#include "frame.h"
#include "font.h"
#include "slider.h"
#include "sprite.h"

static vk_context_t vk;
static vk_render_target_t render_targets[MAX_SCREENS];

static vk_renderer_t *renderers[] =
{
   &frame_renderer,
   &sprite_renderer,
   &font_renderer,
   &slider_renderer,
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
         .pos.values = {10.0, 40.0, 320.0, 64.0},
         .coords.values = {20.0, 120.0, 132.0, 32.0},
         .color.values = {0.4, 1.0, 0.5, 1.20},
      };
      vk_sprite_add(&sprite, &frame_renderer.default_texture);
   }
   {
      sprite_t sprite =
      {
         .pos.values = {10.0, 190.0, 300.0, 300.0},
         .coords.values = {0.0, 0.0, font_renderer.default_texture.width, font_renderer.default_texture.height},
         .color.values = {1.0, 1.0, 1.0, 1.0},
      };
      vk_sprite_add(&sprite, &font_renderer.default_texture);
   }
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

   vk_render_targets_init(&vk, video.screen_count, video.screens, render_targets);

   for (vk_renderer_t **renderer = renderers; *renderer; renderer++)
      if ((*renderer)->init)(*renderer)->init(&vk);

   vk_register_draw_command(&render_targets[0].draw_list, frame_draw);
   vk_register_draw_command(&render_targets[0].draw_list, sprite_test);
   vk_register_draw_command(&render_targets[0].draw_list, fps_draw);
   vk_register_draw_command(&render_targets[0].draw_list, screen_id_draw);

   vk_register_draw_command(&render_targets[1].draw_list, frame_draw_small);
   vk_register_draw_command(&render_targets[1].draw_list, sprite_test);
   vk_register_draw_command(&render_targets[1].draw_list, fps_draw);
   vk_register_draw_command(&render_targets[1].draw_list, screen_id_draw);
   vk_register_draw_command(&render_targets[1].draw_list, console_draw);

   vk_register_draw_command(&render_targets[2].draw_list, frame_draw);
   vk_register_draw_command(&render_targets[2].draw_list, fps_draw);
   vk_register_draw_command(&render_targets[2].draw_list, screen_id_draw);

   test_image.width = 256;
   test_image.height = 256;
   test_image.format = VK_FORMAT_R5G6B5_UNORM_PACK16;
   vk_texture_init(&vk, &test_image);
   memset(test_image.staging.mem.u8 + test_image.staging.mem.layout.offset, 0x80,
      test_image.staging.mem.size - test_image.staging.mem.layout.offset);
}

void video_frame_init(int width, int height, screen_format_t format)
{
   VkFormat vkformat;

   switch (format)
   {
   case screen_format_RGB565:
      vkformat = VK_FORMAT_R5G6B5_UNORM_PACK16;
      break;

   case screen_format_ARGB5551:
      vkformat = VK_FORMAT_R5G5B5A1_UNORM_PACK16;
      break;

   default:
      vkformat = VK_FORMAT_R8G8B8A8_UNORM;
      break;
   }

   vk_frame_init(&vk, width, height, vkformat);
}

void video_frame_update()
{
   uint32_t image_indices[MAX_SCREENS];
   VkCommandBuffer cmds[MAX_SCREENS];
   VkSwapchainKHR swapchains[MAX_SCREENS];

   frame_renderer.default_texture.dirty = true;

//   vkWaitForFences(vk.device, 1, &vk.queue_fence, VK_TRUE, UINT64_MAX);
   VK_CHECK(vkWaitForFences(vk.device, 1, &vk.queue_fence, VK_TRUE, 100000000));
   vkResetFences(vk.device, 1, &vk.queue_fence);

   int i;

   for (i = 0; i < video.screen_count; i++)
   {

      vkWaitForFences(vk.device, 1, &render_targets[i].chain_fence, VK_TRUE, UINT64_MAX);
      vkResetFences(vk.device, 1, &render_targets[i].chain_fence);

      if (vk.vsync != render_targets[i].vsync)
      {
         vk_swapchain_destroy(&vk, &render_targets[i]);
         vk_swapchain_init(&vk, &render_targets[i]);
      }

      while (vkAcquireNextImageKHR(vk.device, render_targets[i].swapchain, UINT64_MAX, NULL,
            render_targets[i].chain_fence, &image_indices[i]) != VK_SUCCESS)
      {
         usleep(100000);
         vk_swapchain_destroy(&vk, &render_targets[i]);
         vk_swapchain_init(&vk, &render_targets[i]);
      }


      //   vkWaitForFences(vk.device, 1, &display_fence, VK_TRUE, UINT64_MAX);
      //   vkResetFences(vk.device, 1, &display_fence);

      {
         const VkCommandBufferBeginInfo info =
         {
            VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
            .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
         };
         vkBeginCommandBuffer(render_targets[i].cmd, &info);
      }


      {
         vk_draw_command_list_t *draw_command = render_targets[i].draw_list;

         while (draw_command)
         {
            draw_command->draw(render_targets[i].screen);
            draw_command = draw_command->next;
         }
      }

      for (vk_renderer_t **renderer = renderers; *renderer; renderer++)
         (*renderer)->update(vk.device, render_targets[i].cmd, *renderer);

      /* renderpass */
      {
         {
//         const VkClearValue clearValue = {{{0.0f, 0.1f, 1.0f, 0.0f}}};
            VkClearValue clearValue = {.color.float32 = {0.0f, 0.1f, 1.0f, 0.0f}};

            const VkRenderPassBeginInfo info =
            {
               VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
               .renderPass = vk.renderpass,
               .framebuffer = render_targets[i].framebuffers[image_indices[i]],
               .renderArea = render_targets[i].scissor,
               .clearValueCount = 1,
               .pClearValues = &clearValue
            };
            vkCmdBeginRenderPass(render_targets[i].cmd, &info, VK_SUBPASS_CONTENTS_INLINE);
         }

         {
            float push_constants[2] = {render_targets[i].viewport.width, render_targets[i].viewport.height};
            vkCmdPushConstants(render_targets[i].cmd, vk.pipeline_layout, VK_SHADER_STAGE_ALL, 0, sizeof(push_constants),
               push_constants);
         }
         vkCmdSetViewport(render_targets[i].cmd, 0, 1, &render_targets[i].viewport);
         vkCmdSetScissor(render_targets[i].cmd, 0, 1, &render_targets[i].scissor);

         for (vk_renderer_t **renderer = renderers; *renderer; renderer++)
            (*renderer)->exec(render_targets[i].cmd, *renderer);

         vkCmdEndRenderPass(render_targets[i].cmd);
      }

      vkEndCommandBuffer(render_targets[i].cmd);
      cmds[i] = render_targets[i].cmd;
      swapchains[i] = render_targets[i].swapchain;
   }

   for (vk_renderer_t **renderer = renderers; *renderer; renderer++)
      (*renderer)->finish(vk.device, *renderer);

   {
      const VkSubmitInfo info =
      {
         VK_STRUCTURE_TYPE_SUBMIT_INFO,
         .commandBufferCount = video.screen_count, cmds
      };
      vkQueueSubmit(vk.queue, 1, &info, vk.queue_fence);
   }

   {
      const VkPresentInfoKHR info =
      {
         VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
         .swapchainCount = video.screen_count,
         .pSwapchains = swapchains,
         .pImageIndices = image_indices
      };
      vkQueuePresentKHR(vk.queue, &info);
   }

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
      vkWaitForFences(vk.device, 1, &render_targets[i].chain_fence, VK_TRUE, UINT64_MAX);

   vkWaitForFences(vk.device, 1, &vk.queue_fence, VK_TRUE, UINT64_MAX);

   for (vk_renderer_t **renderer = renderers; *renderer; renderer++)
      (*renderer)->destroy(vk.device, *renderer);

   vk_texture_free(vk.device, &test_image);
   vk_render_targets_destroy(&vk, video.screen_count, render_targets);
   vk_context_destroy(&vk);

   video.frame.data = NULL;
   debug_log("video destroy\n");
}

void video_toggle_vsync(void)
{
   vk.vsync = !vk.vsync;
}

void video_toggle_filter(void)
{
   frame_renderer.default_texture.filter = !frame_renderer.default_texture.filter;
   frame_renderer.default_texture.info.sampler = frame_renderer.default_texture.filter ? vk.samplers.linear :
      vk.samplers.nearest;
   vkWaitForFences(vk.device, 1, &vk.queue_fence, VK_TRUE, UINT64_MAX);
   vk_texture_update_descriptor_sets(&vk, &frame_renderer.default_texture);
}

const video_t video_vulkan =
{
   .init         = video_init,
   .frame_init   = video_frame_init,
   .frame_update = video_frame_update,
   .destroy      = video_destroy,
   .toggle_vsync = video_toggle_vsync,
   .toggle_filter = video_toggle_filter,
};
