
#include <string.h>
#include <assert.h>

#include "video.h"
#include "common.h"
#include "vulkan_common.h"
#include "frame.h"
#include "font.h"
#include "slider.h"

static vk_context_t vk;
static vk_render_target_t render_targets[MAX_SCREENS];

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

   vulkan_font_init(&vk);
   vulkan_slider_init(&vk);

   render_element_t* el;

   el = render_targets[0].render_elements;
   el->update = (void*)vulkan_frame_update;
   el->render = (void*)vulkan_frame_render;

   el = render_targets[1].render_elements;
   el->update = (void*)vulkan_font_update_assets;
   el->render = (void*)vulkan_font_render;
   el++;
   el->update = (void*)vulkan_slider_update;
   el->render = (void*)vulkan_slider_render;

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

   vulkan_frame_init(&vk, width, height, vkformat);
}

void video_frame_update()
{
   uint32_t image_indices[MAX_SCREENS];
   VkCommandBuffer cmds[MAX_SCREENS];
   VkSwapchainKHR swapchains[MAX_SCREENS];


   vkWaitForFences(vk.device, 1, &vk.queue_fence, VK_TRUE, UINT64_MAX);
   vkResetFences(vk.device, 1, &vk.queue_fence);

//   vulkan_slider_add(400,100, 40, 300, 0.3);
//   vulkan_slider_add(video.screens[1].width - 20, 0, 20, video.screens[1].height, 0.1, 0.2);
//   vulkan_slider_add(video.screens[1].width - 240, 0, 10, 200, 0.3);

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
         render_element_t* el = render_targets[i].render_elements;
         while (el->update)
         {
            el->update(vk.device, render_targets[i].cmd, el->data);
            el++;
         }
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

         {
            render_element_t* el = render_targets[i].render_elements;
            while (el->render)
            {
               el->render(render_targets[i].cmd, el->data);
               el++;
            }

         }

         vkCmdEndRenderPass(render_targets[i].cmd);
      }

      vkEndCommandBuffer(render_targets[i].cmd);
      cmds[i] = render_targets[i].cmd;
      swapchains[i] = render_targets[i].swapchain;
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
   vulkan_slider_destroy(vk.device);
   vulkan_font_destroy(vk.device);
   vulkan_frame_destroy(vk.device);
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
