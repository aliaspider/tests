#pragma once

#include <stdbool.h>

#include "common.h"

static inline VkResult VkAllocateCommandBuffers(VkDevice device, VkCommandPool commandPool, VkCommandBufferLevel level, uint32_t count, VkCommandBuffer *out)
{
   const VkCommandBufferAllocateInfo info =
   {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = level,
      .commandBufferCount = count
   };
   return vkAllocateCommandBuffers(device, &info, out);
}


static inline VkResult VkBeginCommandBuffer(VkCommandBuffer CommandBuffer, VkRenderPass renderPass, VkCommandBufferUsageFlags flags)
{
   VkCommandBufferInheritanceInfo inheritance_info =
   {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_INHERITANCE_INFO,
      .renderPass = renderPass,
   };
   const VkCommandBufferBeginInfo info =
   {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = flags,
      .pInheritanceInfo = &inheritance_info,
   };
   return vkBeginCommandBuffer(CommandBuffer, &info);
}


static inline VkResult VkCreateFence(VkDevice device, bool signaled, VkFence *out)
{
   VkFenceCreateInfo info =
   {
      VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0,
   };
   return vkCreateFence(device, &info, NULL, out);
}
static inline VkResult VkQueueSubmit(VkQueue queue, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers,
                                     VkFence fence)
{
   const VkSubmitInfo info =
   {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = commandBufferCount, pCommandBuffers
   };
   return vkQueueSubmit(queue, 1, &info, fence);
}

static inline VkResult VkQueuePresent(VkQueue queue, uint32_t swapchainCount, const VkSwapchainKHR *pSwapchains,
                                      const uint32_t *pImageIndices)
{
   const VkPresentInfoKHR info =
   {
      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .swapchainCount = swapchainCount,
      .pSwapchains = pSwapchains,
      .pImageIndices = pImageIndices
   };
   return vkQueuePresentKHR(queue, &info);
}

static inline void VkCmdBeginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass,
                                        VkFramebuffer framebuffer,
                                        VkRect2D renderArea, const VkClearValue *pClearValue)
{
   const VkRenderPassBeginInfo info =
   {
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = renderPass,
      .framebuffer = framebuffer,
      .renderArea = renderArea,
      .clearValueCount = 1,
      .pClearValues = pClearValue
   };
   vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
}

