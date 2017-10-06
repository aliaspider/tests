
#include "vulkan_common.h"
#include "display.h"

void surface_init(VkInstance instance, const surface_init_info_t* init_info, surface_t *dst)
{
   /* init display and window */
#ifdef VK_USE_PLATFORM_XLIB_KHR

   /* init surface */
   {
      VkXlibSurfaceCreateInfoKHR info =
      {
         VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
         .dpy = display.display,
         .window = display.window
      };
      vkCreateXlibSurfaceKHR(instance, &info, NULL, &dst->handle);
   }
#else
#error platform not supported
#endif

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
