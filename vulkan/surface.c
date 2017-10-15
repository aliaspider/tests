
#include "vulkan_common.h"


VKAPI_ATTR VkResult VKAPI_CALL vkRegisterDisplayEventEXT(
    VkDevice                                    device,
    VkDisplayKHR                                display,
    const VkDisplayEventInfoEXT*                pDisplayEventInfo,
    const VkAllocationCallbacks*                pAllocator,
    VkFence*                                    pFence)
{
   return VULKAN_CALL_DEV(vkRegisterDisplayEventEXT, device, display, pDisplayEventInfo, pAllocator, pFence);
}

#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
PFN_vkReleaseDisplayEXT pvkReleaseDisplayEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkReleaseDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    VkDisplayKHR                                display)
{
   return pvkReleaseDisplayEXT(physicalDevice, display);
}

PFN_vkAcquireXlibDisplayEXT pvkAcquireXlibDisplayEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkAcquireXlibDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    VkDisplayKHR                                display)
{
   return pvkAcquireXlibDisplayEXT(physicalDevice, dpy, display);
}

PFN_vkGetRandROutputDisplayEXT pvkGetRandROutputDisplayEXT;
VKAPI_ATTR VkResult VKAPI_CALL vkGetRandROutputDisplayEXT(
    VkPhysicalDevice                            physicalDevice,
    Display*                                    dpy,
    RROutput                                    rrOutput,
    VkDisplayKHR*                               pDisplay)
{
   return pvkGetRandROutputDisplayEXT(physicalDevice, dpy, rrOutput, pDisplay);
}

#endif

void surface_init(VkInstance instance, const surface_init_info_t* init_info, surface_t *dst)
{
   /* init display and window */
#ifdef VK_USE_PLATFORM_XLIB_KHR

   /* init surface */
   {
      VkXlibSurfaceCreateInfoKHR info =
      {
         VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
         .dpy = init_info->display,
         .window = init_info->window
      };
      vkCreateXlibSurfaceKHR(instance, &info, NULL, &dst->handle);
   }
#else
#error platform not supported
#endif

   {
      {
         uint32_t displayProperties_count;
         vkGetPhysicalDeviceDisplayPropertiesKHR(init_info->gpu, &displayProperties_count, NULL);
         VkDisplayPropertiesKHR displayProperties[displayProperties_count];
         vkGetPhysicalDeviceDisplayPropertiesKHR(init_info->gpu, &displayProperties_count, displayProperties);
         dst->display = displayProperties[0].display;
         int i;
         for (i = 0; i < displayProperties_count; i++)
         {
            printf("0x%08lX : %s\n", (uintptr_t)displayProperties[i].display, displayProperties[i].displayName);

         }
         vkGetPhysicalDeviceDisplayPlanePropertiesKHR(init_info->gpu, &displayProperties_count, NULL);
         VkDisplayPlanePropertiesKHR displayPlaneProperties[displayProperties_count];
         vkGetPhysicalDeviceDisplayPlanePropertiesKHR(init_info->gpu, &displayProperties_count, displayPlaneProperties);
         for (i = 0; i < displayProperties_count; i++)
         {
            printf("0x%08lX : %i\n", (uintptr_t)displayPlaneProperties[i].currentDisplay, displayPlaneProperties[i].currentStackIndex);

         }
      }
//      uint32_t displayCount = 4;
//      VK_CHECK(vkGetDisplayPlaneSupportedDisplaysKHR(init_info->gpu, 1, &displayCount, &dst->display));


//      VK_CHECK(vkGetRandROutputDisplayEXT(init_info->gpu, init_info->display, 0x27e, &dst->display));
//      VK_CHECK(vkAcquireXlibDisplayEXT(init_info->gpu, init_info->display, dst->display));
//      VK_CHECK(vkReleaseDisplayEXT(init_info->gpu, dst->display));

      uint32_t displayModeCount;
      VK_CHECK(vkGetDisplayModePropertiesKHR(init_info->gpu, dst->display, &displayModeCount, NULL));
      VkDisplayModePropertiesKHR displayModeProperties[displayModeCount];
      VK_CHECK(vkGetDisplayModePropertiesKHR(init_info->gpu, dst->display, &displayModeCount, displayModeProperties));
      printf("displayModeProperties.parameters.refreshRate : %u\n", displayModeProperties[0].parameters.refreshRate);
      printf("displayModeProperties.visibleRegion.width : %u\n", displayModeProperties[0].parameters.visibleRegion.width);
      printf("displayModeProperties.visibleRegion.height : %u\n", displayModeProperties[0].parameters.visibleRegion.height);
   }
   VkSurfaceCapabilitiesKHR surfaceCapabilities;
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(init_info->gpu, dst->handle, &surfaceCapabilities);

   VkBool32 physicalDeviceSurfaceSupport;
   vkGetPhysicalDeviceSurfaceSupportKHR(init_info->gpu, init_info->queue_family_index, dst->handle, &physicalDeviceSurfaceSupport);

   uint32_t surfaceFormatcount;
   vkGetPhysicalDeviceSurfaceFormatsKHR(init_info->gpu, dst->handle, &surfaceFormatcount, NULL);
   VkSurfaceFormatKHR surfaceFormats[surfaceFormatcount];
   vkGetPhysicalDeviceSurfaceFormatsKHR(init_info->gpu, dst->handle, &surfaceFormatcount, surfaceFormats);

   uint32_t presentModeCount;
   vkGetPhysicalDeviceSurfacePresentModesKHR(init_info->gpu, dst->handle, &presentModeCount, NULL);
   VkPresentModeKHR presentModes[presentModeCount];
   vkGetPhysicalDeviceSurfacePresentModesKHR(init_info->gpu, dst->handle, &presentModeCount, presentModes);

   dst->width = init_info->width;
   dst->height = init_info->height;
}

void surface_free(VkInstance instance, surface_t *surface)
{
   vkDestroySurfaceKHR(instance, surface->handle, NULL);
   surface->handle = VK_NULL_HANDLE;
}
