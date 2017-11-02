
#include <string.h>

#include "render_target.h"
#include "inlines.h"

static void vk_get_surface_props(VkPhysicalDevice gpu, uint32_t queue_family_index, VkSurfaceKHR surface)
{
   {
      VkSurfaceCapabilitiesKHR surfaceCapabilities;
      vkGetPhysicalDeviceSurfaceCapabilitiesKHR(gpu, surface, &surfaceCapabilities);
#if 0
      VkSurfaceCapabilities2EXT surfaceCapabilities2 = {VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT};
      vkGetPhysicalDeviceSurfaceCapabilities2EXT(gpu, surface, &surfaceCapabilities2);
      debug_log("surfaceCapabilities2.supportedSurfaceCounters : %i\n", surfaceCapabilities2.supportedSurfaceCounters);
#endif
      VkBool32 physicalDeviceSurfaceSupport;
      vkGetPhysicalDeviceSurfaceSupportKHR(gpu, queue_family_index, surface, &physicalDeviceSurfaceSupport);

      uint32_t surfaceFormatcount;
      vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surfaceFormatcount, NULL);
      VkSurfaceFormatKHR surfaceFormats[surfaceFormatcount];
      vkGetPhysicalDeviceSurfaceFormatsKHR(gpu, surface, &surfaceFormatcount, surfaceFormats);

      uint32_t presentModeCount;
      vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, NULL);
      VkPresentModeKHR presentModes[presentModeCount];
      vkGetPhysicalDeviceSurfacePresentModesKHR(gpu, surface, &presentModeCount, presentModes);
      int i;

      for (i = 0; i < presentModeCount; i++)
         debug_log("supports present mode %i\n", presentModes[i]);

      fflush(stdout);
   }
}

void vk_swapchain_init(vk_context_t *vk, vk_render_target_t *render_target)
{
   VkSurfaceCapabilitiesKHR surfaceCapabilities;
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk->gpu, render_target->surface, &surfaceCapabilities);
   render_target->screen->width = surfaceCapabilities.currentExtent.width;
   render_target->screen->height = surfaceCapabilities.currentExtent.height;
   render_target->vsync = video.vsync;

   {
//      VkSwapchainCounterCreateInfoEXT swapchainCounterCreateInfo =
//      {
//         VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT,
//         .surfaceCounters = VK_SURFACE_COUNTER_VBLANK_EXT
//      };
#ifdef WIN32
#define VK_PRESENT_MODE_IMMEDIATE_KHR VK_PRESENT_MODE_MAILBOX_KHR
#endif
      VkSwapchainCreateInfoKHR info =
      {
         VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
//         .pNext = &swapchainCounterCreateInfo,
         .surface = render_target->surface,
         .minImageCount = 2,
         .imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
         .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
         .imageExtent.width = render_target->screen->width,
         .imageExtent.height = render_target->screen->height,
         .imageArrayLayers = 1,
         .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
         .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
         .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
         .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
         .presentMode = render_target->vsync ? VK_PRESENT_MODE_FIFO_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR,
//         .presentMode = render_target->vsync? VK_PRESENT_MODE_FIFO_RELAXED_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR,
//         .presentMode = render_target->vsync? VK_PRESENT_MODE_MAILBOX_KHR : VK_PRESENT_MODE_IMMEDIATE_KHR,
         .clipped = VK_TRUE
      };
      VK_CHECK(vkCreateSwapchainKHR(vk->device, &info, NULL, &render_target->swapchain));
   }

   render_target->viewport.x = 0.0f;
   render_target->viewport.y = 0.0f;
   render_target->viewport.width = render_target->screen->width;
   render_target->viewport.height = render_target->screen->height;
   render_target->viewport.minDepth = -1.0f;
   render_target->viewport.maxDepth =  1.0f;

   render_target->scissor.offset.x = 0.0f;
   render_target->scissor.offset.y = 0.0f;
   render_target->scissor.extent.width = render_target->screen->width;
   render_target->scissor.extent.height = render_target->screen->height;

   render_target->clear_value.color = (VkClearColorValue)
   {
      {
         0.0f, 0.1f, 1.0f, 0.0f
      }
   };


   {
      VkAllocateCommandBuffers(vk->device, vk->pools.cmd, VK_COMMAND_BUFFER_LEVEL_SECONDARY, 1, &render_target->cmd);

      VkBeginCommandBuffer(render_target->cmd, vk->renderpass, VK_RENDER_PASS_CONTINUE);

      vkCmdPushConstants(render_target->cmd, vk->pipeline_layout, VK_SHADER_STAGE_ALL, 0, 2 * sizeof(float),
                         &render_target->viewport.width);
      vkCmdSetViewport(render_target->cmd, 0, 1, &render_target->viewport);
      vkCmdSetScissor(render_target->cmd, 0, 1, &render_target->scissor);

      vkEndCommandBuffer(render_target->cmd);
   }

   {
      vkGetSwapchainImagesKHR(vk->device, render_target->swapchain, &render_target->swapchain_count, NULL);

      if (render_target->swapchain_count > MAX_SWAPCHAIN_IMAGES)
         render_target->swapchain_count = MAX_SWAPCHAIN_IMAGES;

      VkImage swapchainImages[render_target->swapchain_count];
      vkGetSwapchainImagesKHR(vk->device, render_target->swapchain, &render_target->swapchain_count,
                              swapchainImages);

      int i;

      for (i = 0; i < render_target->swapchain_count; i++)
      {
         {
            VkImageViewCreateInfo info =
            {
               VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
               .image = swapchainImages[i],
               .viewType = VK_IMAGE_VIEW_TYPE_2D,
               .format = VK_FORMAT_B8G8R8A8_UNORM,
               .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
               .subresourceRange.levelCount = 1,
               .subresourceRange.layerCount = 1
            };
            vkCreateImageView(vk->device, &info, NULL, &render_target->views[i]);
         }

         {
            VkFramebufferCreateInfo info =
            {
               VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
               .renderPass = vk->renderpass,
               .attachmentCount = 1, &render_target->views[i],
               .width = render_target->screen->width,
               .height = render_target->screen->height,
               .layers = 1
            };
            vkCreateFramebuffer(vk->device, &info, NULL, &render_target->framebuffers[i]);
         }
         {
            VkRenderPassBeginInfo info =
            {
               VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
               .renderPass = vk->renderpass,
               .framebuffer = render_target->framebuffers[i],
               .renderArea = render_target->scissor,
               .clearValueCount = 1,
               .pClearValues = &render_target->clear_value
            };
            render_target->renderpass_info[i] = info;
         }
      }
   }
}

void vk_render_targets_init(vk_context_t *vk, int count, screen_t *screens, vk_render_target_t *render_targets)
{
   int i;

   for (i = 0; i < count; i++)
   {
      render_targets[i].screen = &screens[i];

#ifdef VK_USE_PLATFORM_XLIB_KHR
      VkXlibSurfaceCreateInfoKHR info =
      {
         VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
         .dpy = render_targets[i].screen->display,
         .window = render_targets[i].screen->window
      };
      vkCreateXlibSurfaceKHR(vk->instance, &info, NULL, &render_targets[i].surface);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
      VkWin32SurfaceCreateInfoKHR info =
      {
         VK_STRUCTURE_TYPE_WIN32_SURFACE_CREATE_INFO_KHR,
         .hinstance = render_targets[i].screen->hinstance,
         .hwnd = render_targets[i].screen->hwnd,
      };
      vkCreateWin32SurfaceKHR(vk->instance, &info, NULL, &render_targets[i].surface);
#else
#error platform not supported
#endif

      vk_get_surface_props(vk->gpu, vk->queue_family_index, render_targets[i].surface);

      vk_swapchain_init(vk, &render_targets[i]);

      VkCreateFence(vk->device, true, &render_targets[i].chain_fence);

//   {
//      VkDisplayEventInfoEXT displayEventInfo =
//      {
//         VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT,
//         .displayEvent = VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT
//      };
//      VK_CHECK(vkRegisterDisplayEventEXT(vk->device, surface.display, &displayEventInfo, NULL, &display_fence));
//   }
   }

}

void vk_swapchain_destroy(vk_context_t *vk, vk_render_target_t *render_target)
{

   int i;

   for (i = 0; i < render_target->swapchain_count; i++)
   {
      vkDestroyImageView(vk->device, render_target->views[i], NULL);
      vkDestroyFramebuffer(vk->device, render_target->framebuffers[i], NULL);
   }

   vkFreeCommandBuffers(vk->device, vk->pools.cmd, 1, &render_target->cmd);
   vkDestroySwapchainKHR(vk->device, render_target->swapchain, NULL);

}

void vk_render_targets_destroy(vk_context_t *vk, int count, vk_render_target_t *render_targets)
{

//   vkWaitForFences(vk->device, 1, &display_fence, VK_TRUE, UINT64_MAX);
//   vkDestroyFence(vk->device, display_fence, NULL);

   int i;

   for (i = 0; i < count; i++)
   {
      vkWaitForFences(vk->device, 1, &render_targets[i].chain_fence, VK_TRUE, UINT64_MAX);
      vkDestroyFence(vk->device, render_targets[i].chain_fence, NULL);
      vk_swapchain_destroy(vk, &render_targets[i]);
      vkDestroySurfaceKHR(vk->instance, render_targets[i].surface, NULL);
   }

   memset(render_targets, 0, count * sizeof(*render_targets));


}
