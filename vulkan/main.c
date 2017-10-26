
#include <string.h>
#include <assert.h>

#include "common.h"
#include "video.h"
#include "input.h"
#include "vulkan_common.h"
#include "frame.h"
#include "font.h"
#include "slider.h"

static vk_context_t vk;
static vk_render_target_t render_targets[MAX_SCREENS];

static vk_renderer_t* renderers[] =
{
   &frame_renderer,
   &font_renderer,
   &slider_renderer,
   NULL
};
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

   vk_font_init(&vk);
   vk_slider_init(&vk);
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

   frame_renderer.texture.dirty = true;

   vkWaitForFences(vk.device, 1, &vk.queue_fence, VK_TRUE, UINT64_MAX);
   vkResetFences(vk.device, 1, &vk.queue_fence);

   int i;

   for (i = 0; i < video.screen_count; i++)
   {
      vkWaitForFences(vk.device, 1, &render_targets[i].chain_fence, VK_TRUE, UINT64_MAX);
      vkResetFences(vk.device, 1, &render_targets[i].chain_fence);
      vkAcquireNextImageKHR(vk.device, render_targets[i].swapchain, UINT64_MAX, NULL, render_targets[i].chain_fence,
         &image_indices[i]);


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
         vk_renderer_t** renderer = renderers;
         while(*renderer)
            vk_renderer_update(vk.device, render_targets[i].cmd, *renderer++);
      }

      /* renderpass */
      {
         {
//         const VkClearValue clearValue = {{{0.0f, 0.1f, 1.0f, 0.0f}}};
            const VkClearValue clearValue = {.color.float32 = {0.0f, 0.1f, 1.0f, 0.0f}};

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

         char buffer[512];
         snprintf(buffer, sizeof(buffer), "SCREEN: %i", i);
         font_render_options_t options =
         {
            .x = video.screens[0].width - 20 - strlen(buffer) * 12,
            .max_width = video.screens[0].width,
            .max_height = video.screens[0].height,
         };
         vk_font_draw_text(buffer, &options);

         if (i == 0)
            vk_frame_add(0, 0, render_targets[i].viewport.width, render_targets[i].viewport.height);
         else if (i == 1)
         {
            void console_draw(void);
            console_draw();

            font_render_options_t options =
            {
               .max_width = video.screens[0].width,
               .max_height = video.screens[0].height,
            };

            char buffer[512];
            vk_font_draw_text(video.fps, &options);

            snprintf(buffer, sizeof(buffer), "[%c,%c,%c] \e[91m%i, \e[32m%i", input.pointer.touch1 ? '#' : ' ',
               input.pointer.touch2 ? '#' : ' ', input.pointer.touch3 ? '#' : ' ', input.pointer.x, input.pointer.y);
            options.y = 20;
            vk_font_draw_text(buffer, &options);

//            static int text_pos_y = 100;

//            vulkan_font_draw_text("Backward compatibility: Backwards compatibility with ASCII and the enormous "
//               "amount of software designed to process ASCII-encoded text was the main driving "
//               "force behind the design of UTF-8. In UTF-8, single bytes with values in the range "
//               "of 0 to 127 map directly to Unicode code points in the ASCII range. Single bytes "
//               "in this range represent characters, as they do in ASCII.\n\nMoreover, 7-bit bytes "
//               "(bytes where the most significant bit is 0) never appear in a multi-byte sequence, "
//               "and no valid multi-byte sequence decodes to an ASCII code-point. A sequence of 7-bit "
//               "bytes is both valid ASCII and valid UTF-8, and under either interpretation represents "
//               "the same sequence of characters.\n\nTherefore, the 7-bit bytes in a UTF-8 stream represent "
//               "all and only the ASCII characters in the stream. Thus, many text processors, parsers, "
//               "protocols, file formats, text display programs etc., which use ASCII characters for "
//               "formatting and control purposes will continue to work as intended by treating the UTF-8 "
//               "byte stream as a sequence of single-byte characters, without decoding the multi-byte sequences. "
//               "ASCII characters on which the processing turns, such as punctuation, whitespace, and control "
//               "characters will never be encoded as multi-byte sequences. It is therefore safe for such "
//               "processors to simply ignore or pass-through the multi-byte sequences, without decoding them. "
//               "For example, ASCII whitespace may be used to tokenize a UTF-8 stream into words; "
//               "ASCII line-feeds may be used to split a UTF-8 stream into lines; and ASCII NUL ", 0, text_pos_y,
//               video.screens[0].width);

//            vulkan_font_draw_text("北海道の有名なかん光地、知床半島で、黒いキツネがさつえいされました。"
//               "地元斜里町の町立知床博物館が、タヌキをかんさつするためにおいていた自動さつえいカメラがき重なすがたをとらえました＝"
//               "写真・同館ていきょう。同館の学芸員も「はじめて見た」とおどろいています。"
//               "黒い毛皮のために昔、ゆ入したキツネの子そんとも言われてますが、はっきりしません。"
//               "北海道の先住みん族、アイヌのみん話にも黒いキツネが登場し、神せいな生き物とされているそうです。",
//               0, text_pos_y, video.screens[0].width);

//            vulkan_font_draw_text("gl_Position.xy = pos + 2.0 * vec2(0.0, glyph_metrics[c].w) / vp_size;", 0, 32,
//               video.screens[0].width);

//            vulkan_font_draw_text("ettf", 40, 220, video.screens[0].width);

         }

         {
            vk_renderer_t** renderer = renderers;
            while(*renderer)
               vk_renderer_draw(render_targets[i].cmd, *renderer++);
         }
//         {
//            render_element_t* el = render_targets[i].render_elements;
//            while (el->render)
//            {
//               el->render(render_targets[i].cmd, el->data);
//               el++;
//            }

//         }

         vkCmdEndRenderPass(render_targets[i].cmd);
      }

      vkEndCommandBuffer(render_targets[i].cmd);
      cmds[i] = render_targets[i].cmd;
      swapchains[i] = render_targets[i].swapchain;
   }

   {
      vk_renderer_t** renderer = renderers;
      while(*renderer)
         vk_renderer_finish(vk.device, *renderer++);
   }

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
   vk_slider_destroy(vk.device);
   vk_font_destroy(vk.device);
   vk_frame_destroy(vk.device);
   vk_render_targets_destroy(&vk, video.screen_count, render_targets);
   vk_context_destroy(&vk);

   video.frame.data = NULL;
   debug_log("video destroy\n");
}

const video_t video_vulkan =
{
   .init         = video_init,
   .frame_init   = video_frame_init,
   .frame_update = video_frame_update,
   .destroy      = video_destroy
};
