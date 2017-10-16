
#include "vulkan_common.h"


#define VK_INST_FN_LIST    \
   VK_FN(vkCreateDebugReportCallbackEXT);\
   VK_FN(vkDestroyDebugReportCallbackEXT);\
   VK_FN(vkCreateDebugReportCallbackEXT);\
   VK_FN(vkDestroyDebugReportCallbackEXT);\
   VK_FN(vkRegisterDisplayEventEXT);\
   VK_FN(vkGetPhysicalDeviceSurfaceCapabilities2EXT);\
   VK_FN(vkReleaseDisplayEXT);\
   VK_FN(vkAcquireXlibDisplayEXT);\
   VK_FN(vkGetRandROutputDisplayEXT);

#define VK_DEV_FN_LIST    \
   VK_FN(vkGetRefreshCycleDurationGOOGLE);\
   VK_FN(vkGetPastPresentationTimingGOOGLE);\
   VK_FN(vkGetSwapchainCounterEXT);

#define VK_FN(fn)     PFN_##fn fn##p

VK_INST_FN_LIST
VK_DEV_FN_LIST

#undef VK_FN


void vk_init_instance_pfn(VkInstance instance)
{
#define VK_FN(fn)     fn##p = (PFN_##fn)vkGetInstanceProcAddr(instance, #fn);
   VK_INST_FN_LIST
#undef VK_FN
}

void vk_init_device_pfn(VkDevice device)
{
#define VK_FN(fn)      fn##p = (PFN_##fn)vkGetDeviceProcAddr(device, #fn);
   VK_DEV_FN_LIST
#undef VK_FN
}


VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
   VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback)
{
   return vkCreateDebugReportCallbackEXTp(instance, pCreateInfo, pAllocator, pCallback);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,const VkAllocationCallbacks *pAllocator)
{
   return vkDestroyDebugReportCallbackEXTp(instance, callback, pAllocator);
}


VKAPI_ATTR VkResult VKAPI_CALL vkRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence)
{
   return vkRegisterDisplayEventEXTp(device, display, pDisplayEventInfo, pAllocator, pFence);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilities2EXT(
    VkPhysicalDevice                            physicalDevice,
    VkSurfaceKHR                                surface,
    VkSurfaceCapabilities2EXT*                  pSurfaceCapabilities)
{
   return vkGetPhysicalDeviceSurfaceCapabilities2EXTp(physicalDevice, surface, pSurfaceCapabilities);
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT

VKAPI_ATTR VkResult VKAPI_CALL vkReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display)
{
   return vkReleaseDisplayEXTp(physicalDevice, display);
}

VKAPI_ATTR VkResult VKAPI_CALL vkAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display)
{
   return vkAcquireXlibDisplayEXTp(physicalDevice, dpy, display);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay)
{
   return vkGetRandROutputDisplayEXTp(physicalDevice, dpy, rrOutput, pDisplay);
}

#endif

VKAPI_ATTR VkResult VKAPI_CALL vkGetRefreshCycleDurationGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkRefreshCycleDurationGOOGLE*               pDisplayTimingProperties)
{
   return vkGetRefreshCycleDurationGOOGLEp(device, swapchain, pDisplayTimingProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPastPresentationTimingGOOGLE(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    uint32_t*                                   pPresentationTimingCount,
    VkPastPresentationTimingGOOGLE*             pPresentationTimings)
{
   return vkGetPastPresentationTimingGOOGLEp(device, swapchain, pPresentationTimingCount, pPresentationTimings);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainCounterEXT(
    VkDevice                                    device,
    VkSwapchainKHR                              swapchain,
    VkSurfaceCounterFlagBitsEXT                 counter,
    uint64_t*                                   pCounterValue)
{
   return vkGetSwapchainCounterEXTp(device, swapchain, counter, pCounterValue);

}
