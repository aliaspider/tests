
#include <stdio.h>
#include <assert.h>
#include <vulkan/vulkan_common.h>

#include "vulkan_common.h"

#ifdef HAVE_X11
#include "video.h"
#endif

VKAPI_ATTR VkResult VKAPI_CALL vkCreateDebugReportCallbackEXT(
   VkInstance instance, const VkDebugReportCallbackCreateInfoEXT *pCreateInfo,
      const VkAllocationCallbacks *pAllocator, VkDebugReportCallbackEXT *pCallback)
{
   return VULKAN_CALL(vkCreateDebugReportCallbackEXT, instance, pCreateInfo, pAllocator, pCallback);
}

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,const VkAllocationCallbacks *pAllocator)
{
   return VULKAN_CALL(vkDestroyDebugReportCallbackEXT, instance, callback, pAllocator);
}


void instance_init(instance_t* dst)
{
   {
      const char *layers[] =
      {
#ifdef __WIN32__
//         "VK_LAYER_LUNARG_api_dump",
         "VK_LAYER_LUNARG_core_validation",
//         "VK_LAYER_LUNARG_device_simulation",
//         "VK_LAYER_LUNARG_monitor",
         "VK_LAYER_LUNARG_object_tracker",
         "VK_LAYER_LUNARG_parameter_validation",
//         "VK_LAYER_LUNARG_screenshot",
         "VK_LAYER_LUNARG_standard_validation",
         "VK_LAYER_GOOGLE_threading",
         "VK_LAYER_GOOGLE_unique_objects",
//         "VK_LAYER_LUNARG_vktrace",
//         "VK_LAYER_NV_optimus",
//         "VK_LAYER_VALVE_steam_overlay",
//         "VK_LAYER_RENDERDOC_Capture"
#else
//         "VK_LAYER_LUNARG_standard_validation",
//         "VK_LAYER_GOOGLE_unique_objects",
//         "VK_LAYER_LUNARG_core_validation",
//         "VK_LAYER_LUNARG_object_tracker",
//         "VK_LAYER_LUNARG_parameter_validation",
//         "VK_LAYER_GOOGLE_threading"
#endif
      };
      const char *instance_ext[] =
      {
         VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
         VK_KHR_SURFACE_EXTENSION_NAME,
         VK_KHR_DISPLAY_EXTENSION_NAME,
//         VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
         VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME,
#ifdef VK_USE_PLATFORM_WIN32_KHR
         VK_KHR_WIN32_SURFACE_EXTENSION_NAME,
#endif
#ifdef VK_USE_PLATFORM_XLIB_KHR
         VK_KHR_XLIB_SURFACE_EXTENSION_NAME,         
#endif
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
         VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
         VK_EXT_ACQUIRE_XLIB_DISPLAY_EXTENSION_NAME,
#endif
      };
#if 1
      {
         int l,e;
         uint32_t lprop_count;
         vkEnumerateInstanceLayerProperties(&lprop_count, NULL);
         VkLayerProperties lprops[lprop_count + 1];
         vkEnumerateInstanceLayerProperties(&lprop_count, lprops);
         lprops[lprop_count].layerName[0] = '\0';
         for (l = 0; l < lprop_count + 1; l++)
         {
            uint32_t iexprop_count;
            vkEnumerateInstanceExtensionProperties(lprops[l].layerName, &iexprop_count, NULL);
            VkExtensionProperties iexprops[iexprop_count];
            vkEnumerateInstanceExtensionProperties(lprops[l].layerName, &iexprop_count, iexprops);
            printf("%s (%i)\n", lprops[l].layerName, iexprop_count);
            for(e = 0; e < iexprop_count; e++)
            {
               printf("\t%s\n", iexprops[e].extensionName);
            }
         }
         fflush(stdout);
      }
#endif

      VkApplicationInfo appinfo =
      {
         VK_STRUCTURE_TYPE_APPLICATION_INFO,
         .pApplicationName = "Vulkan Test",
         .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
         .pEngineName = "my engine",
         .engineVersion = VK_MAKE_VERSION(0, 1, 0)
      };

      VkInstanceCreateInfo info =
      {
         VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
         .pApplicationInfo = &appinfo,
         .enabledLayerCount = countof(layers),
         .ppEnabledLayerNames = layers,
         .enabledExtensionCount = countof(instance_ext),
         .ppEnabledExtensionNames = instance_ext
      };
      VK_CHECK(vkCreateInstance(&info, NULL, &dst->handle));
   }

//   {
//      VkDebugReportCallbackCreateInfoEXT info =
//      {
//         VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
//         .flags =
//         VK_DEBUG_REPORT_ERROR_BIT_EXT |
//         VK_DEBUG_REPORT_WARNING_BIT_EXT |
//         //      VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
//         //      VK_DEBUG_REPORT_DEBUG_BIT_EXT |
//         VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
//         .pfnCallback = vulkan_debug_report_callback,
//         .pUserData = NULL
//      };
//      vkCreateDebugReportCallbackEXT(dst->handle, &info, NULL, &dst->debug_cb);
//   }

   pvkGetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)vkGetInstanceProcAddr(dst->handle, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
   pvkReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)vkGetInstanceProcAddr(dst->handle, "vkReleaseDisplayEXT");
   pvkAcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)vkGetInstanceProcAddr(dst->handle, "vkAcquireXlibDisplayEXT");
   pvkGetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)vkGetInstanceProcAddr(dst->handle, "vkGetRandROutputDisplayEXT");
#endif


}

void instance_free(instance_t* instance)
{
   vkDestroyDebugReportCallbackEXT(instance->handle, instance->debug_cb, NULL);
   vkDestroyInstance(instance->handle, NULL);

   instance->debug_cb = VK_NULL_HANDLE;
   instance->handle = VK_NULL_HANDLE;
}
