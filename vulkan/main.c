
#include <string.h>

#include "video.h"
#include "common.h"
#include "vulkan_common.h"
#include "frame.h"
#include "font.h"

video_t video;

typedef struct
{
   instance_t instance;
   physical_device_t gpu;
   device_t dev;
   surface_t surface;
   swapchain_t chain;
   vk_descriptor_t desc;
   VkCommandBuffer cmd;
   VkFence queue_fence;
   VkFence chain_fence;
} vk_context_t;

static instance_t instance;
static physical_device_t gpu;
static device_t dev;
static surface_t surface;
static swapchain_t chain;
//static vk_texture_t tex;
//static vk_buffer_t vbo;
//static buffer_t  ubo;
static vk_descriptor_t desc;
//static VkDescriptorSet frame_desc;
//static vk_pipeline_t pipe;
static VkCommandBuffer cmd;
static VkFence queue_fence;
static VkFence chain_fence;
//static VkFence display_fence;

void video_init()
{
   debug_log("video init\n");
   instance_init(&instance);

   physical_device_init(instance.handle, &gpu);

   device_init(gpu.handle, &dev);

   video.screen.width = 640;
   video.screen.height = 480;
#ifdef VK_USE_PLATFORM_XLIB_KHR
   video.screen.display = XOpenDisplay(NULL);
   video.screen.window  = XCreateSimpleWindow(video.screen.display,
                          DefaultRootWindow(video.screen.display), 0, 0, video.screen.width, video.screen.height, 0, 0, 0);
   XStoreName(video.screen.display, video.screen.window, "Vulkan Test");
   XSelectInput(video.screen.display, video.screen.window, ExposureMask | FocusChangeMask | KeyPressMask | KeyReleaseMask);
   XMapWindow(video.screen.display, video.screen.window);
#endif

   {
      surface_init_info_t info =
      {
         .gpu = gpu.handle,
         .queue_family_index = dev.queue_family_index,
         .width = video.screen.width,
         .height = video.screen.height,
#ifdef VK_USE_PLATFORM_XLIB_KHR
         .display = video.screen.display,
         .window = video.screen.window
#endif
      };
      surface_init(instance.handle, &info, &surface);
   }

   {
      swapchain_init_info_t info =
      {
         .surface = surface.handle,
         .width = surface.width,
         .height = surface.height,
         .present_mode = VK_PRESENT_MODE_FIFO_KHR
         //         .present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR
      };
      swapchain_init(dev.handle, &info, &chain);
   }


   descriptors_init(dev.handle, &desc);


   {
      const VkCommandBufferAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
         .commandPool = dev.cmd_pool,
         .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
         .commandBufferCount = 1
      };
      vkAllocateCommandBuffers(dev.handle, &info, &cmd);
   }

   {
      VkFenceCreateInfo info =
      {
         VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
         .flags = VK_FENCE_CREATE_SIGNALED_BIT
      };
      vkCreateFence(dev.handle, &info, NULL, &chain_fence);
      vkCreateFence(dev.handle, &info, NULL, &queue_fence);
   }

//   {
//      VkDisplayEventInfoEXT displayEventInfo =
//      {
//         VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT,
//         .displayEvent = VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT
//      };
//      VK_CHECK(vkRegisterDisplayEventEXT(dev.handle, surface.display, &displayEventInfo, NULL, &display_fence));
//   }

   //   uniforms_t *uniforms = ubo.mem.ptr;
   //   uniforms->center.x = 0.0f;
   //   uniforms->center.y = 0.0f;
   //   uniforms->image.width  = tex.width;
   //   uniforms->image.height = tex.height;
   //   uniforms->screen.width  = chain.viewport.width;
   //   uniforms->screen.height = chain.viewport.height;
   //   uniforms->angle = 0.0f;

   vulkan_font_init(dev.handle, dev.queue_family_index, gpu.memoryTypes, &desc, &chain.scissor, &chain.viewport, chain.renderpass);

}

void video_frame_update()
{
   uint32_t image_index;
   vkWaitForFences(dev.handle, 1, &chain_fence, VK_TRUE, -1);
   vkResetFences(dev.handle, 1, &chain_fence);
   vkAcquireNextImageKHR(dev.handle, chain.handle, UINT64_MAX, NULL, chain_fence, &image_index);

   vkWaitForFences(dev.handle, 1, &queue_fence, VK_TRUE, -1);
   vkResetFences(dev.handle, 1, &queue_fence);

//   vkWaitForFences(dev.handle, 1, &display_fence, VK_TRUE, -1);
//   vkResetFences(dev.handle, 1, &display_fence);

   {
      const VkCommandBufferBeginInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
      };
      vkBeginCommandBuffer(cmd, &info);
   }

   vulkan_frame_update(dev.handle, cmd);
   vulkan_font_update_assets(cmd);

   /* renderpass */
   {
      {
         const VkClearValue clearValue = {{{0.0f, 0.1f, 1.0f, 0.0f}}};
         const VkRenderPassBeginInfo info =
         {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = chain.renderpass,
            .framebuffer = chain.framebuffers[image_index],
            .renderArea = chain.scissor,
            .clearValueCount = 1,
            .pClearValues = &clearValue
         };
         vkCmdBeginRenderPass(cmd, &info, VK_SUBPASS_CONTENTS_INLINE);
      }

      vulkan_frame_render(cmd);
//      vulkan_font_render(cmd);

      vkCmdEndRenderPass(cmd);
   }

   vkEndCommandBuffer(cmd);

   {
      const VkSubmitInfo info =
      {
         VK_STRUCTURE_TYPE_SUBMIT_INFO,
         .commandBufferCount = 1,
         .pCommandBuffers = &cmd
      };
      vkQueueSubmit(dev.queue, 1, &info, queue_fence);
   }

   {
      const VkPresentInfoKHR info =
      {
         VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
         .swapchainCount = 1,
         .pSwapchains = &chain.handle,
         .pImageIndices = &image_index
      };
      vkQueuePresentKHR(dev.queue, &info);
   }
#if 0
   {
      uint64_t vblank_counter = 0;
      VK_CHECK(vkGetSwapchainCounterEXT(dev.handle, chain.handle, VK_SURFACE_COUNTER_VBLANK_EXT, &vblank_counter));
      printf("vblank_counter : %"PRId64"\n", vblank_counter);
   }
#endif
}

void video_destroy()
{

   vkWaitForFences(dev.handle, 1, &queue_fence, VK_TRUE, UINT64_MAX);
   vkDestroyFence(dev.handle, queue_fence, NULL);

   vkWaitForFences(dev.handle, 1, &chain_fence, VK_TRUE, UINT64_MAX);
   vkDestroyFence(dev.handle, chain_fence, NULL);

//   vkWaitForFences(dev.handle, 1, &display_fence, VK_TRUE, UINT64_MAX);
//   vkDestroyFence(dev.handle, display_fence, NULL);

   vulkan_font_destroy(dev.handle, desc.pool);
   vulkan_frame_destroy(dev.handle);

   descriptors_free(dev.handle, &desc);
   //   buffer_free(dev.handle, &ubo);
   swapchain_free(dev.handle, &chain);
   surface_free(instance.handle, &surface);
   device_free(&dev);
   physical_device_free(&gpu);
   instance_free(&instance);

#ifdef HAVE_X11
   XDestroyWindow(video.screen.display, video.screen.window);
   XCloseDisplay(video.screen.display);
   video.screen.display = NULL;
#endif

   video.frame.data = NULL;
   debug_log("video destroy\n");
}

void video_frame_init(int width, int height, screen_format_t format)
{
   vulkan_frame_init(dev.handle, dev.queue_family_index, gpu.memoryTypes, &desc, width, height,
                     format == screen_format_RGB565 ? VK_FORMAT_R5G6B5_UNORM_PACK16 :
                                                      format == screen_format_ARGB5551 ? VK_FORMAT_R5G5B5A1_UNORM_PACK16 :
                                                                                         VK_FORMAT_R8G8B8A8_SRGB,
                     &chain.scissor, &chain.viewport, chain.renderpass);


}

const video_t video_vulkan =
{
   .init = video_init,
   .frame_init = video_frame_init,
   .frame_update = video_frame_update,
   .destroy = video_destroy
};
