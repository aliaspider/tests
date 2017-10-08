
#include "vulkan_common.h"

void swapchain_init(VkDevice device, const swapchain_init_info_t *init_info, swapchain_t* dst)
{
   VkSwapchainCreateInfoKHR swapchainCreateInfo =
   {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .surface = init_info->surface,
      .minImageCount = 2,
      .imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
      .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      .imageExtent.width = init_info->width,
      .imageExtent.height = init_info->height,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = init_info->present_mode,
      .clipped = VK_TRUE
   };
   vkCreateSwapchainKHR(device, &swapchainCreateInfo, NULL, &dst->handle);

   /* init renderpass */
   VkAttachmentDescription attachmentDescriptions[] =
   {
      {
         0, VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT,
         VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
         VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
         VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
      }
   };

   VkAttachmentReference ColorAttachment = {0,  VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL};

   VkSubpassDescription subpassDescriptions[] =
   {
      {
         .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
         .colorAttachmentCount = 1, &ColorAttachment,
      }
   };

   VkRenderPassCreateInfo renderPassCreateInfo =
   {
      VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
      .attachmentCount = countof(attachmentDescriptions), attachmentDescriptions,
      .subpassCount = countof(subpassDescriptions), subpassDescriptions,
   };

   vkCreateRenderPass(device, &renderPassCreateInfo, NULL, &dst->renderpass);

   /* init image views and framebuffers */
   vkGetSwapchainImagesKHR(device, dst->handle, &dst->count, NULL);
   if(dst->count > MAX_SWAPCHAIN_IMAGES)
      dst->count = MAX_SWAPCHAIN_IMAGES;

   VkImage swapchainImages[dst->count];
   vkGetSwapchainImagesKHR(device, dst->handle, &dst->count, swapchainImages);


   int i;
   for (i = 0; i < dst->count; i++)
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
         vkCreateImageView(device, &info, NULL, &dst->views[i]);
      }

      {
         VkFramebufferCreateInfo info =
         {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = dst->renderpass,
            .attachmentCount = 1, &dst->views[i],
            .width = init_info->width,
            .height = init_info->height,
            .layers = 1
         };
         vkCreateFramebuffer(device, &info, NULL, &dst->framebuffers[i]);
      }
   }

   dst->viewport.x = 0.0f;
   dst->viewport.y = 0.0f;
   dst->viewport.width = init_info->width;
   dst->viewport.height = init_info->height;
   dst->viewport.minDepth = -1.0f;
   dst->viewport.maxDepth =  1.0f;

   dst->scissor.offset.x = 0.0f;
   dst->scissor.offset.y = 0.0f;
   dst->scissor.extent.width = init_info->width;
   dst->scissor.extent.height = init_info->height;
}

void swapchain_free(VkDevice device, swapchain_t *chain)
{
   int i;
   for (i = 0; i < chain->count; i++)
   {
      vkDestroyImageView(device, chain->views[i], NULL);
      vkDestroyFramebuffer(device, chain->framebuffers[i], NULL);
      chain->views[i] = VK_NULL_HANDLE;
      chain->framebuffers[i] = VK_NULL_HANDLE;
   }
   vkDestroySwapchainKHR(device, chain->handle, NULL);
   vkDestroyRenderPass(device, chain->renderpass, NULL);

   chain->handle = VK_NULL_HANDLE;
   chain->renderpass = VK_NULL_HANDLE;
}
