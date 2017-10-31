
#include <string.h>
#include <assert.h>

#include "vulkan_common.h"
#include "common.h"

#define VK_INST_FN_LIST    \
   VK_FN(vkCreateDebugReportCallbackEXT);\
   VK_FN(vkDestroyDebugReportCallbackEXT);\
   VK_FN(vkCreateDebugReportCallbackEXT);\
   VK_FN(vkDestroyDebugReportCallbackEXT);\
   VK_FN(vkRegisterDisplayEventEXT);\
   VK_FN(vkGetPhysicalDeviceSurfaceCapabilities2EXT);\
   VK_FN(vkReleaseDisplayEXT);

#ifdef VK_USE_PLATFORM_XLIB_KHR
#define VK_PLATFORM_FN_LIST \
   VK_FN(vkAcquireXlibDisplayEXT); \
   VK_FN(vkGetRandROutputDisplayEXT);
#elif defined(VK_USE_PLATFORM_WIN32_KHR)
#else
#endif

#define VK_DEV_FN_LIST    \
   VK_FN(vkGetRefreshCycleDurationGOOGLE);\
   VK_FN(vkGetPastPresentationTimingGOOGLE);\
   VK_FN(vkGetSwapchainCounterEXT);

#define VK_FN(fn)     static PFN_##fn fn##p

VK_INST_FN_LIST
VK_PLATFORM_FN_LIST
VK_DEV_FN_LIST

#undef VK_FN


static void vk_init_instance_pfn(VkInstance instance)
{
#define VK_FN(fn)     fn##p = (PFN_##fn)vkGetInstanceProcAddr(instance, #fn);
   VK_INST_FN_LIST
   VK_PLATFORM_FN_LIST
#undef VK_FN
}

static void vk_init_device_pfn(VkDevice device)
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

VKAPI_ATTR void VKAPI_CALL vkDestroyDebugReportCallbackEXT(VkInstance instance, VkDebugReportCallbackEXT callback,
      const VkAllocationCallbacks *pAllocator)
{
   return vkDestroyDebugReportCallbackEXTp(instance, callback, pAllocator);
}


VKAPI_ATTR VkResult VKAPI_CALL vkRegisterDisplayEventEXT(
   VkDevice                                    device,
   VkDisplayKHR                                display,
   const VkDisplayEventInfoEXT                *pDisplayEventInfo,
   const VkAllocationCallbacks                *pAllocator,
   VkFence                                    *pFence)
{
   return vkRegisterDisplayEventEXTp(device, display, pDisplayEventInfo, pAllocator, pFence);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPhysicalDeviceSurfaceCapabilities2EXT(
   VkPhysicalDevice                            physicalDevice,
   VkSurfaceKHR                                surface,
   VkSurfaceCapabilities2EXT                  *pSurfaceCapabilities)
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
   Display                                    *dpy,
   VkDisplayKHR                                display)
{
   return vkAcquireXlibDisplayEXTp(physicalDevice, dpy, display);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetRandROutputDisplayEXT(
   VkPhysicalDevice                            physicalDevice,
   Display                                    *dpy,
   RROutput                                    rrOutput,
   VkDisplayKHR                               *pDisplay)
{
   return vkGetRandROutputDisplayEXTp(physicalDevice, dpy, rrOutput, pDisplay);
}

#endif

VKAPI_ATTR VkResult VKAPI_CALL vkGetRefreshCycleDurationGOOGLE(
   VkDevice                                    device,
   VkSwapchainKHR                              swapchain,
   VkRefreshCycleDurationGOOGLE               *pDisplayTimingProperties)
{
   return vkGetRefreshCycleDurationGOOGLEp(device, swapchain, pDisplayTimingProperties);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetPastPresentationTimingGOOGLE(
   VkDevice                                    device,
   VkSwapchainKHR                              swapchain,
   uint32_t                                   *pPresentationTimingCount,
   VkPastPresentationTimingGOOGLE             *pPresentationTimings)
{
   return vkGetPastPresentationTimingGOOGLEp(device, swapchain, pPresentationTimingCount, pPresentationTimings);
}

VKAPI_ATTR VkResult VKAPI_CALL vkGetSwapchainCounterEXT(
   VkDevice                                    device,
   VkSwapchainKHR                              swapchain,
   VkSurfaceCounterFlagBitsEXT                 counter,
   uint64_t                                   *pCounterValue)
{
   return vkGetSwapchainCounterEXTp(device, swapchain, counter, pCounterValue);

}

static void vk_get_instance_props(void)
{
   uint32_t lprop_count;
   vkEnumerateInstanceLayerProperties(&lprop_count, NULL);
   VkLayerProperties lprops[lprop_count + 1];
   vkEnumerateInstanceLayerProperties(&lprop_count, lprops);
   lprops[lprop_count].layerName[0] = '\0';

   int l;

   for (l = 0; l < lprop_count + 1; l++)
   {
      uint32_t iexprop_count;
      vkEnumerateInstanceExtensionProperties(lprops[l].layerName, &iexprop_count, NULL);
      VkExtensionProperties iexprops[iexprop_count];
      vkEnumerateInstanceExtensionProperties(lprops[l].layerName, &iexprop_count, iexprops);
      debug_log("%s (%i)\n", lprops[l].layerName, iexprop_count);

      int e;

      for (e = 0; e < iexprop_count; e++)
         debug_log("\t%s\n", iexprops[e].extensionName);
   }

   fflush(stdout);
}

static void vk_get_gpu_props(VkPhysicalDevice gpu)
{
   VkPhysicalDeviceProperties gpu_props;
   vkGetPhysicalDeviceProperties(gpu, &gpu_props);

   uint32_t deviceExtensionPropertiesCount;
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &deviceExtensionPropertiesCount, NULL);

   VkExtensionProperties pDeviceExtensionProperties[deviceExtensionPropertiesCount];
   vkEnumerateDeviceExtensionProperties(gpu, NULL, &deviceExtensionPropertiesCount,
                                        pDeviceExtensionProperties);

   int e;

   for (e = 0; e < deviceExtensionPropertiesCount; e++)
      debug_log("\t%s\n", pDeviceExtensionProperties[e].extensionName);

#if 0
   {
      uint32_t displayProperties_count;
      vkGetPhysicalDeviceDisplayPropertiesKHR(vk->gpu, &displayProperties_count, NULL);
      VkDisplayPropertiesKHR displayProperties[displayProperties_count];
      vkGetPhysicalDeviceDisplayPropertiesKHR(vk->gpu, &displayProperties_count, displayProperties);
      dst->display = displayProperties[0].display;
      int i;

      for (i = 0; i < displayProperties_count; i++)
         debug_log("0x%08" PRIXPTR " : %s\n", (uintptr_t)displayProperties[i].display,
                   displayProperties[i].displayName);

   }
   vkGetPhysicalDeviceDisplayPlanePropertiesKHR(vk->gpu, &displayProperties_count, NULL);
   VkDisplayPlanePropertiesKHR displayPlaneProperties[displayProperties_count];
   vkGetPhysicalDeviceDisplayPlanePropertiesKHR(vk->gpu, &displayProperties_count, displayPlaneProperties);

   for (i = 0; i < displayProperties_count; i++)
      debug_log("0x%08" PRIXPTR " : %i\n", (uintptr_t)displayPlaneProperties[i].currentDisplay,
                displayPlaneProperties[i].currentStackIndex);

//   uint32_t displayCount = 4;
//   VK_CHECK(vkGetDisplayPlaneSupportedDisplaysKHR(vk->gpu, 1, &displayCount, &dst->display));


//   VK_CHECK(vkGetRandROutputDisplayEXT(vk->gpu, init_info->display, 0x27e, &dst->display));
//   VK_CHECK(vkAcquireXlibDisplayEXT(vk->gpu, init_info->display, dst->display));
//   VK_CHECK(vkReleaseDisplayEXT(vk->gpu, dst->display));
//   exit(0);


   uint32_t                   displayModeCount;
   VK_CHECK(vkGetDisplayModePropertiesKHR(vk->gpu, dst->display, &displayModeCount, NULL));
   VkDisplayModePropertiesKHR displayModeProperties[displayModeCount];
   VK_CHECK(vkGetDisplayModePropertiesKHR(vk->gpu, dst->display, &displayModeCount, displayModeProperties));
   debug_log("displayModeProperties.parameters.refreshRate : %u\n", displayModeProperties[0].parameters.refreshRate);
   debug_log("displayModeProperties.visibleRegion.width : %u\n", displayModeProperties[0].parameters.visibleRegion.width);
   debug_log("displayModeProperties.visibleRegion.height : %u\n",
             displayModeProperties[0].parameters.visibleRegion.height);
#endif


   fflush(stdout);
}

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

static uint32_t vk_get_queue_family_index(VkPhysicalDevice gpu, VkQueueFlags required_flags)
{
   uint32_t queueFamilyPropertyCount;
   vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertyCount, NULL);

   VkQueueFamilyProperties pQueueFamilyProperties[queueFamilyPropertyCount];
   vkGetPhysicalDeviceQueueFamilyProperties(gpu, &queueFamilyPropertyCount, pQueueFamilyProperties);

   int i;

   for (i = 0; i < queueFamilyPropertyCount; i++)
      if ((pQueueFamilyProperties[i].queueFlags & required_flags) == required_flags)
         return i;

   return 0;
}

static VkBool32 vulkan_debug_report_callback(VkDebugReportFlagsEXT flags,
      VkDebugReportObjectTypeEXT objectType,
      uint64_t object, size_t location,
      int32_t messageCode,
      const char *pLayerPrefix,
      const char *pMessage, void *pUserData)
{
   static const char *debugFlags_str[] = {"INFORMATION", "WARNING", "PERFORMANCE", "ERROR", "DEBUG"};

   // ignore "vkAcquireNextImageKHR: Application has already acquired the maximum number of images (0x1)"
   if (objectType == VK_DEBUG_REPORT_OBJECT_TYPE_SWAPCHAIN_KHR_EXT && messageCode == 108)
      return VK_FALSE;

   if (false
         || messageCode == 61
//   || messageCode == 68
//      || messageCode == 38
//      || messageCode == 438304791
//      || messageCode == 9
      )
      return VK_FALSE;

   int i;

   for (i = 0; i < countof(debugFlags_str); i++)
      if (flags & (1 << i))
         break;

   debug_log("[%-14s - %-11s] %s\n", pLayerPrefix, debugFlags_str[i], pMessage);
   fflush(stdout);

   assert((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) == 0);
   return VK_FALSE;
}

void vk_context_init(vk_context_t *vk)
{

   {
      static const char *layers[] =
      {
         "VK_LAYER_LUNARG_standard_validation",
         "VK_LAYER_GOOGLE_unique_objects",
         "VK_LAYER_LUNARG_core_validation",
         "VK_LAYER_LUNARG_object_tracker",
         "VK_LAYER_LUNARG_parameter_validation",
         "VK_LAYER_GOOGLE_threading"
#ifdef __WIN32__
//         "VK_LAYER_LUNARG_api_dump",
//         "VK_LAYER_LUNARG_device_simulation",
//         "VK_LAYER_LUNARG_monitor",
//         "VK_LAYER_LUNARG_screenshot",
//         "VK_LAYER_LUNARG_vktrace",
//         "VK_LAYER_NV_optimus",
//         "VK_LAYER_VALVE_steam_overlay",
//         "VK_LAYER_RENDERDOC_Capture"
#endif
      };
      static const char *instance_ext[] =
      {
         VK_EXT_DEBUG_REPORT_EXTENSION_NAME,
         VK_KHR_SURFACE_EXTENSION_NAME,
//         VK_EXT_DIRECT_MODE_DISPLAY_EXTENSION_NAME,
         VK_EXT_DISPLAY_SURFACE_COUNTER_EXTENSION_NAME,
#ifdef __linux__
         VK_KHR_DISPLAY_EXTENSION_NAME,
#endif
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

      vk_get_instance_props();

      static const VkApplicationInfo appinfo =
      {
         VK_STRUCTURE_TYPE_APPLICATION_INFO,
         .pApplicationName = "Vulkan Test",
         .applicationVersion = VK_MAKE_VERSION(0, 1, 0),
         .pEngineName = "my engine",
         .engineVersion = VK_MAKE_VERSION(0, 1, 0)
      };

      static const VkInstanceCreateInfo info =
      {
         VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
         .pApplicationInfo = &appinfo,
         .enabledLayerCount = countof(layers),
         .ppEnabledLayerNames = layers,
         .enabledExtensionCount = countof(instance_ext),
         .ppEnabledExtensionNames = instance_ext
      };
      VK_CHECK(vkCreateInstance(&info, NULL, &vk->instance));
   }

   vk_init_instance_pfn(vk->instance);

   {
      static const VkDebugReportCallbackCreateInfoEXT info =
      {
         VK_STRUCTURE_TYPE_DEBUG_REPORT_CALLBACK_CREATE_INFO_EXT,
         .flags =
         VK_DEBUG_REPORT_ERROR_BIT_EXT |
         VK_DEBUG_REPORT_WARNING_BIT_EXT |
//      VK_DEBUG_REPORT_INFORMATION_BIT_EXT |
//      VK_DEBUG_REPORT_DEBUG_BIT_EXT |
         VK_DEBUG_REPORT_PERFORMANCE_WARNING_BIT_EXT,
         .pfnCallback = vulkan_debug_report_callback,
         .pUserData = NULL
      };
      vkCreateDebugReportCallbackEXT(vk->instance, &info, NULL, &vk->debug_cb);
   }

   {
      uint32_t one = 1;
      vkEnumeratePhysicalDevices(vk->instance, &one, &vk->gpu);
   }

   vkGetPhysicalDeviceMemoryProperties(vk->gpu, &vk->mem);

   vk_get_gpu_props(vk->gpu);

   vk->queue_family_index = vk_get_queue_family_index(vk->gpu, VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT);

   {
      static const float one = 1.0;
      static const char *device_ext[] =
      {
         VK_KHR_SWAPCHAIN_EXTENSION_NAME,
#ifdef __linux__
         VK_EXT_DISPLAY_CONTROL_EXTENSION_NAME,
#endif
//         VK_KHR_DISPLAY_SWAPCHAIN_EXTENSION_NAME,
      };

      const VkDeviceQueueCreateInfo queue_info =
      {
         VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
         .queueFamilyIndex = vk->queue_family_index,
         .queueCount = 1,
         .pQueuePriorities = &one
      };

      static const VkPhysicalDeviceFeatures enabledFeatures =
      {
         .geometryShader = VK_TRUE,
         .samplerAnisotropy = VK_TRUE,
         .vertexPipelineStoresAndAtomics = VK_TRUE
      };

      const VkDeviceCreateInfo info =
      {
         VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
         .queueCreateInfoCount = 1,
         .pQueueCreateInfos = &queue_info,
         .enabledExtensionCount = countof(device_ext),
         .ppEnabledExtensionNames = device_ext,
         .pEnabledFeatures = &enabledFeatures
      };
      vkCreateDevice(vk->gpu, &info, NULL, &vk->device);
   }

   vk_init_device_pfn(vk->device);

   vkGetDeviceQueue(vk->device, vk->queue_family_index, 0, &vk->queue);

   {
      VkCommandPoolCreateInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
         .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         .queueFamilyIndex = vk->queue_family_index
      };
      vkCreateCommandPool(vk->device, &info, NULL, &vk->pools.cmd);
   }

   {
      static const VkDescriptorPoolSize sizes[] =
      {
         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC, 32},
         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 32},
         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 32},
         {VK_DESCRIPTOR_TYPE_STORAGE_BUFFER, 32}
      };

      static const VkDescriptorPoolCreateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
         .maxSets = 32,
         .poolSizeCount = countof(sizes), sizes
      };
      vkCreateDescriptorPool(vk->device, &info, NULL, &vk->pools.desc);
   }

   {
      VkSamplerCreateInfo info =
      {
         VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
         .magFilter = VK_FILTER_NEAREST,
         .minFilter = VK_FILTER_NEAREST,
         .addressModeU = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
         .addressModeV = VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE,
         .unnormalizedCoordinates = VK_TRUE
      };
      vkCreateSampler(vk->device, &info, NULL, &vk->samplers.nearest);

      info.magFilter = VK_FILTER_LINEAR;
      info.minFilter = VK_FILTER_LINEAR;
      vkCreateSampler(vk->device, &info, NULL, &vk->samplers.linear);
   }


   {
      const VkDescriptorSetLayoutBinding bindings[] =
      {
         {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
         },
         {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 2,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = (VkSampler *) &vk->samplers
         },
         {
            .binding = 2,
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_GEOMETRY_BIT,
         }
      };

      VkDescriptorSetLayoutCreateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
         .bindingCount = countof(bindings), bindings

      };
      vkCreateDescriptorSetLayout(vk->device, &info, NULL, &vk->set_layouts.base);
   }

   {
      static const VkDescriptorSetLayoutBinding bindings[] =
      {
         {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
         {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
         },
      };

      static const VkDescriptorSetLayoutCreateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
         .bindingCount = countof(bindings), bindings

      };
      vkCreateDescriptorSetLayout(vk->device, &info, NULL, &vk->set_layouts.texture);
   }

   {
      static const VkPushConstantRange ranges[] =
      {
         {
            .stageFlags = VK_SHADER_STAGE_ALL,
            .offset = 0,
            .size = 8
         },
      };

      const VkPipelineLayoutCreateInfo info =
      {
         VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
         .setLayoutCount = sizeof(vk->set_layouts) / sizeof(VkDescriptorSetLayout),
         .pSetLayouts = (VkDescriptorSetLayout *) &vk->set_layouts,
         .pushConstantRangeCount = countof(ranges), ranges
      };

      vkCreatePipelineLayout(vk->device, &info, NULL, &vk->pipeline_layout);
   }

   {
      static const VkAttachmentDescription attachmentDescriptions[] =
      {
         {
            0, VK_FORMAT_B8G8R8A8_UNORM, VK_SAMPLE_COUNT_1_BIT,
            VK_ATTACHMENT_LOAD_OP_CLEAR, VK_ATTACHMENT_STORE_OP_STORE,
            VK_ATTACHMENT_LOAD_OP_DONT_CARE, VK_ATTACHMENT_STORE_OP_DONT_CARE,
            VK_IMAGE_LAYOUT_UNDEFINED, VK_IMAGE_LAYOUT_PRESENT_SRC_KHR
         }
      };

      static const VkAttachmentReference ColorAttachment =
      {
         0,
         VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL
      };

      static const VkSubpassDescription subpassDescriptions[] =
      {
         {
            .pipelineBindPoint = VK_PIPELINE_BIND_POINT_GRAPHICS,
            .colorAttachmentCount = 1, &ColorAttachment,
         }
      };

      static const VkRenderPassCreateInfo renderPassCreateInfo =
      {
         VK_STRUCTURE_TYPE_RENDER_PASS_CREATE_INFO,
         .attachmentCount = countof(attachmentDescriptions), attachmentDescriptions,
         .subpassCount = countof(subpassDescriptions), subpassDescriptions,
      };

      vkCreateRenderPass(vk->device, &renderPassCreateInfo, NULL, &vk->renderpass);
   }

   VkCreateFence(vk->device, true, &vk->queue_fence);
}

void vk_context_destroy(vk_context_t *vk)
{
   vkWaitForFences(vk->device, 1, &vk->queue_fence, VK_TRUE, UINT64_MAX);
   vkDestroyFence(vk->device, vk->queue_fence, NULL);
   vkDestroyDescriptorPool(vk->device, vk->pools.desc, NULL);
   vkDestroySampler(vk->device, vk->samplers.nearest, NULL);
   vkDestroySampler(vk->device, vk->samplers.linear, NULL);
   vkDestroyDescriptorSetLayout(vk->device, vk->set_layouts.base, NULL);
   vkDestroyDescriptorSetLayout(vk->device, vk->set_layouts.texture, NULL);
   vkDestroyPipelineLayout(vk->device, vk->pipeline_layout, NULL);
   vkDestroyRenderPass(vk->device, vk->renderpass, NULL);
   vkDestroyCommandPool(vk->device, vk->pools.cmd, NULL);
   vkDestroyDevice(vk->device, NULL);
   vkDestroyDebugReportCallbackEXT(vk->instance, vk->debug_cb, NULL);
   vkDestroyInstance(vk->instance, NULL);
   memset(vk, 0, sizeof(*vk));
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
      }
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
void vk_texture_update_descriptor_sets(vk_context_t *vk, vk_texture_t *out)

{
   const VkWriteDescriptorSet write_set[] =
   {
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = out->desc,
         .dstBinding = 0,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .pImageInfo = &out->info
      },
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = out->desc,
         .dstBinding = 1,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
         .pBufferInfo = &out->ubo.info,
      }
   };

   vkUpdateDescriptorSets(vk->device, countof(write_set), write_set, 0, NULL);
}

void vk_texture_init(vk_context_t *vk, vk_texture_t *out)
{
   out->info.sampler = out->filter == VK_FILTER_LINEAR ? vk->samplers.linear : vk->samplers.nearest;
   out->info.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   out->staging.format = out->format;

   {
      VkImageCreateInfo info =
      {
         VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
         .flags = VK_IMAGE_CREATE_MUTABLE_FORMAT_BIT,
         .imageType = VK_IMAGE_TYPE_2D,
         .format = out->format,
         .extent.width = out->width,
         .extent.height = out->height,
         .extent.depth = 1,
         .mipLevels = 1,
         .arrayLayers = 1,
         .samples = VK_SAMPLE_COUNT_1_BIT,
         .tiling = VK_IMAGE_TILING_OPTIMAL,
         .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
         .sharingMode = VK_SHARING_MODE_EXCLUSIVE,
         .queueFamilyIndexCount = 1,
         .pQueueFamilyIndices = &vk->queue_family_index,
         .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
      };

      VK_CHECK(vkCreateImage(vk->device, &info, NULL, &out->image));

      info.tiling = VK_IMAGE_TILING_LINEAR;
      info.initialLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
      info.usage = VK_IMAGE_USAGE_TRANSFER_SRC_BIT;
      out->staging.layout = VK_IMAGE_LAYOUT_PREINITIALIZED;
      VK_CHECK(vkCreateImage(vk->device, &info, NULL, &out->staging.image));
   }

   {
      static const VkImageSubresource imageSubresource =
      {
         .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .mipLevel = 0,
         .arrayLayer = 0
      };
      vkGetImageSubresourceLayout(vk->device, out->staging.image, &imageSubresource, &out->staging.mem.layout);
   }

   {
      memory_init_info_t info =
      {
         .req_flags = VK_MEMORY_PROPERTY_DEVICE_LOCAL_BIT,
         .image = out->image
      };
      vk_device_memory_init(vk->device, vk->memoryTypes, &info, &out->mem);
   }
   {
      memory_init_info_t info =
      {
         .req_flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT,
         .image = out->staging.image
      };
      vk_device_memory_init(vk->device, vk->memoryTypes, &info, &out->staging.mem);
   }

   {
      VkImageViewCreateInfo info =
      {
         VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
         .image = out->image,
         .viewType = VK_IMAGE_VIEW_TYPE_2D,
         .format = out->format,
         .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .subresourceRange.levelCount = 1,
         .subresourceRange.layerCount = 1
      };
      VK_CHECK(vkCreateImageView(vk->device, &info, NULL, &out->info.imageView));
   }

   out->ubo.info.range = 256;
   out->ubo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   out->ubo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
   vk_buffer_init(vk->device, vk->memoryTypes, NULL, &out->ubo);
   out->uniforms = out->ubo.mem.ptr;
   out->uniforms->format = out->format;
   out->uniforms->ignore_alpha = out->ignore_alpha;
   out->ubo.dirty = true;

   {
      const VkDescriptorSetAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .descriptorPool = vk->pools.desc,
         .descriptorSetCount = 1, &vk->set_layouts.texture
      };
      VK_CHECK(vkAllocateDescriptorSets(vk->device, &info, &out->desc));
   }

   vk_texture_update_descriptor_sets(vk, out);

}

void vk_texture_free(VkDevice device, vk_texture_t *texture)
{
   vkDestroyImageView(device, texture->info.imageView, NULL);
   vkDestroyImage(device, texture->image, NULL);
   vkDestroyImage(device, texture->staging.image, NULL);
   vk_device_memory_free(device, &texture->mem);
   vk_device_memory_free(device, &texture->staging.mem);
   texture->info.imageView = VK_NULL_HANDLE;
   texture->image = VK_NULL_HANDLE;
}

void vk_texture_flush(VkDevice device, vk_texture_t *texture)
{
   vk_device_memory_flush(device, &texture->staging.mem);
   texture->flushed = true;

   if (texture->flushed && texture->uploaded)
      texture->dirty = false;
}

void vk_texture_upload(VkDevice device, VkCommandBuffer cmd, vk_texture_t *texture)
{
   VkImageMemoryBarrier barrier =
   {
      VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER,
      .srcQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .dstQueueFamilyIndex = VK_QUEUE_FAMILY_IGNORED,
      .subresourceRange.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
      .subresourceRange.levelCount = 1,
      .subresourceRange.layerCount = 1
   };

   if (texture->staging.layout == VK_IMAGE_LAYOUT_PREINITIALIZED)
   {
      barrier.srcAccessMask = VK_ACCESS_HOST_WRITE_BIT;
      barrier.dstAccessMask = VK_ACCESS_TRANSFER_READ_BIT;
      barrier.oldLayout = VK_IMAGE_LAYOUT_PREINITIALIZED;
      barrier.newLayout = VK_IMAGE_LAYOUT_GENERAL;
      barrier.image  = texture->staging.image;
      vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_HOST_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                           0, 0, NULL, 0, NULL, 1, &barrier);

      texture->staging.layout = VK_IMAGE_LAYOUT_GENERAL;
   }

   barrier.srcAccessMask = 0;
   barrier.dstAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
   barrier.oldLayout = VK_IMAGE_LAYOUT_UNDEFINED;
   barrier.newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
   barrier.image  = texture->image;
   vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TOP_OF_PIPE_BIT, VK_PIPELINE_STAGE_TRANSFER_BIT,
                        0, 0, NULL, 0, NULL, 1, &barrier);

   if (texture->format == texture->staging.format)
   {
      const VkImageCopy copy =
      {
         .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .srcSubresource.layerCount = 1,
         .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .dstSubresource.layerCount = 1,
         .extent.width = texture->width,
         .extent.height = texture->height,
         .extent.depth = 1
      };
      vkCmdCopyImage(cmd, texture->staging.image, VK_IMAGE_LAYOUT_GENERAL,
                     texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     1, &copy);
   }
   else
   {
      const VkImageBlit blit =
      {
         .srcSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .srcSubresource.layerCount = 1,
         .dstSubresource.aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
         .dstSubresource.layerCount = 1,
         .srcOffsets = {{0, 0, 0}, {texture->width, texture->height, 1}},
         .dstOffsets = {{0, 0, 0}, {texture->width, texture->height, 1}}
      };
      vkCmdBlitImage(cmd, texture->staging.image, VK_IMAGE_LAYOUT_GENERAL,
                     texture->image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
                     1, &blit, VK_FILTER_NEAREST);
   }

   barrier.srcAccessMask = VK_ACCESS_TRANSFER_WRITE_BIT;
   barrier.dstAccessMask = VK_ACCESS_SHADER_READ_BIT;
   barrier.oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL;
   barrier.newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
   vkCmdPipelineBarrier(cmd, VK_PIPELINE_STAGE_TRANSFER_BIT, VK_PIPELINE_STAGE_FRAGMENT_SHADER_BIT,
                        0, 0, NULL, 0, NULL, 1, &barrier);

   texture->uploaded = true;

   if (texture->flushed && texture->uploaded)
      texture->dirty = false;
}


void vk_device_memory_init(VkDevice device, const VkMemoryType *memory_types, const memory_init_info_t *init_info,
                           device_memory_t *out)
{

   VkMemoryRequirements reqs;

   if (init_info->buffer)
      vkGetBufferMemoryRequirements(device, init_info->buffer, &reqs);
   else
      vkGetImageMemoryRequirements(device, init_info->image, &reqs);

   out->size = reqs.size;
   out->alignment = reqs.alignment;

   const VkMemoryType *type = memory_types;
   {
      uint32_t bits = reqs.memoryTypeBits;

      while (bits)
      {
         if ((bits & 1) && ((type->propertyFlags & init_info->req_flags) == init_info->req_flags))
            break;

         bits >>= 1;
         type++;
      }
   }

   out->flags = type->propertyFlags;

   {
      const VkMemoryAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_MEMORY_ALLOCATE_INFO,
         .allocationSize = out->size,
         .memoryTypeIndex = type - memory_types
      };
      VK_CHECK(vkAllocateMemory(device, &info, NULL, &out->handle));
   }

   if (init_info->req_flags & VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT)
      vkMapMemory(device, out->handle, 0, out->size, 0, &out->ptr);
   else
      out->ptr = NULL;

   if (init_info->buffer)
      vkBindBufferMemory(device, init_info->buffer, out->handle, 0);
   else
      vkBindImageMemory(device, init_info->image, out->handle, 0);
}

void vk_device_memory_free(VkDevice device, device_memory_t *memory)
{
   if (memory->ptr)
      vkUnmapMemory(device, memory->handle);

   vkFreeMemory(device, memory->handle, NULL);

   memory->ptr = NULL;
   memory->flags = 0;
   memory->handle = VK_NULL_HANDLE;
}

void vk_device_memory_flush(VkDevice device, const device_memory_t *memory)
{
   if (memory->flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
      return;

   {
      VkMappedMemoryRange range =
      {
         VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
         .memory = memory->handle,
         .offset = 0,
         .size = memory->size
      };
      vkFlushMappedMemoryRanges(device, 1, &range);
   }
}

void device_memory_invalidate(VkDevice device, const device_memory_t *memory)
{
   if (memory->flags & VK_MEMORY_PROPERTY_HOST_COHERENT_BIT)
      return;

   {
      VkMappedMemoryRange range =
      {
         VK_STRUCTURE_TYPE_MAPPED_MEMORY_RANGE,
         .memory = memory->handle,
         .offset = 0,
         .size = memory->size
      };
      vkInvalidateMappedMemoryRanges(device, 1, &range);
   }
}

void vk_buffer_init(VkDevice device, const VkMemoryType *memory_types, const void *data, vk_buffer_t *out)
{
   {
      const VkBufferCreateInfo info =
      {
         VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
         .size = out->info.range,
         .usage = out->usage,
      };
      vkCreateBuffer(device, &info, NULL, &out->info.buffer);
   }

   {
      memory_init_info_t info =
      {
         .req_flags = out->mem.flags,
         .buffer = out->info.buffer
      };
      vk_device_memory_init(device, memory_types, &info, &out->mem);
   }

   if (data && out->mem.ptr)
   {
      memcpy(out->mem.ptr, data, out->info.range);
      vk_device_memory_flush(device, &out->mem);
   }
}

void vk_buffer_free(VkDevice device, vk_buffer_t *buffer)
{
   vk_device_memory_free(device, &buffer->mem);
   vkDestroyBuffer(device, buffer->info.buffer, NULL);
   buffer->info.buffer = VK_NULL_HANDLE;
}

void vk_buffer_flush(VkDevice device, vk_buffer_t *buffer)
{
   vk_device_memory_flush(device, &buffer->mem);
   buffer->dirty = false;
}

void vk_buffer_invalidate(VkDevice device, vk_buffer_t *buffer)
{
   device_memory_invalidate(device, &buffer->mem);
   buffer->dirty = false;
}

static inline VkShaderModule vk_shader_code_init(VkDevice device, const vk_shader_code_t *shader_code)
{
   VkShaderModule shader;

   const VkShaderModuleCreateInfo info =
   {
      VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
      .codeSize = shader_code->code_size,
      .pCode = shader_code->code
   };
   vkCreateShaderModule(device, &info, NULL, &shader);

   return shader;
}

static void vk_update_descriptor_sets(vk_context_t *vk, vk_renderer_t *out)
{
   VkWriteDescriptorSet write_set[3];
   int write_set_count = 0;

   if (out->ubo.info.buffer)
   {
//      dst->ubo.info.range =VK_WHOLE_SIZE;
      VkWriteDescriptorSet set =
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = out->desc,
         .dstBinding = 0,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER_DYNAMIC,
         .pBufferInfo = &out->ubo.info
      };
      write_set[write_set_count++] = set;
   }

   VkDescriptorImageInfo image_info[] =
   {
      {
         .sampler = out->tex.info.sampler,
         .imageView = out->tex.info.imageView,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      },
      {
         .sampler = out->tex.info.sampler,
         .imageView = out->tex.info.imageView,
         .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
      }
   };

   if (out->tex.image)
   {
      VkWriteDescriptorSet set =
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = out->desc,
         .dstBinding = 1,
         .dstArrayElement = 0,
         .descriptorCount = countof(image_info),
         .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
         .pImageInfo = image_info
      };
      write_set[write_set_count++] = set;
   }

   if (out->ssbo.info.buffer)
   {
      VkWriteDescriptorSet set =
      {
         VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
         .dstSet = out->desc,
         .dstBinding = 2,
         .dstArrayElement = 0,
         .descriptorCount = 1,
         .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER,
         .pBufferInfo = &out->ssbo.info
      };
      write_set[write_set_count++] = set;
   }

   vkUpdateDescriptorSets(vk->device, write_set_count, write_set, 0, NULL);
}

void vk_renderer_init(vk_context_t *vk, const vk_renderer_init_info_t *init_info, vk_renderer_t *out)
{
   if (out->tex.image)
      out->tex.is_reference = true;
   else if (out->tex.format != VK_FORMAT_UNDEFINED)
      vk_texture_init(vk, &out->tex);

   if (out->ssbo.info.range)
   {
      out->ssbo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      out->ssbo.usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
      vk_buffer_init(vk->device, vk->memoryTypes, NULL, &out->ssbo);
   }

   if (out->ubo.info.range)
   {
      out->ubo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
      out->ubo.usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
      vk_buffer_init(vk->device, vk->memoryTypes, NULL, &out->ubo);
   }

   out->vbo.mem.flags = VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT;
   out->vbo.usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT;
   vk_buffer_init(vk->device, vk->memoryTypes, NULL, &out->vbo);
   out->vbo.info.range = 0;

   {
      const VkDescriptorSetAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .descriptorPool = vk->pools.desc,
         .descriptorSetCount = 1, &vk->set_layouts.base
      };
      vkAllocateDescriptorSets(vk->device, &info, &out->desc);
   }

   vk_update_descriptor_sets(vk, out);


   out->layout = vk->pipeline_layout;

   {
      const VkPipelineShaderStageCreateInfo shaders_info[] =
      {
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .pName = "main",
            .module = vk_shader_code_init(vk->device, &init_info->shaders.vs)
         },
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pName = "main",
            .module = vk_shader_code_init(vk->device, &init_info->shaders.ps)
         },
         {
            VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_GEOMETRY_BIT,
            .pName = "main",
            .module = init_info->shaders.gs.code ? vk_shader_code_init(vk->device, &init_info->shaders.gs) : VK_NULL_HANDLE
         }
      };

      const VkVertexInputBindingDescription vertex_description =
      {
         0, out->vertex_stride, VK_VERTEX_INPUT_RATE_VERTEX
      };

      const VkPipelineVertexInputStateCreateInfo vertex_input_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
         .vertexBindingDescriptionCount = 1, &vertex_description,
         .vertexAttributeDescriptionCount = init_info->attrib_count, init_info->attrib_desc
      };

      const VkPipelineInputAssemblyStateCreateInfo input_assembly_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
         .topology = init_info->topology,
         .primitiveRestartEnable = VK_FALSE
      };

      static const VkPipelineViewportStateCreateInfo viewport_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
         .viewportCount = 1, .scissorCount = 1
      };

      static const VkPipelineRasterizationStateCreateInfo rasterization_info =
      {
         VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
//         .rasterizerDiscardEnable = VK_TRUE,
         .lineWidth = 1.0f
      };

      static const VkPipelineMultisampleStateCreateInfo multisample_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
         .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT,
      };

      const VkPipelineColorBlendStateCreateInfo colorblend_state =
      {
         VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
         .attachmentCount = 1, init_info->color_blend_attachement_state
      };

      static const VkDynamicState dynamic_states[] = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
      static const VkPipelineDynamicStateCreateInfo dynamic_state_info =
      {
         VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
         .dynamicStateCount = countof(dynamic_states), dynamic_states
      };

      VkGraphicsPipelineCreateInfo info[MAX_SCREENS] =
      {
         {
            VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
            .flags = VK_PIPELINE_CREATE_DISABLE_OPTIMIZATION_BIT | VK_PIPELINE_CREATE_ALLOW_DERIVATIVES_BIT,
            .stageCount = shaders_info[2].module ? 3 : 2, shaders_info,
            .pVertexInputState = &vertex_input_state,
            .pInputAssemblyState = &input_assembly_state,
            .pViewportState = &viewport_state,
            .pRasterizationState = &rasterization_info,
            .pMultisampleState = &multisample_state,
            .pColorBlendState = &colorblend_state,
            .pDynamicState = &dynamic_state_info,
            .layout = out->layout,
            .renderPass = vk->renderpass,
            .subpass = 0
         }
      };
      vkCreateGraphicsPipelines(vk->device, VK_NULL_HANDLE, 1, info, NULL, &out->pipe);

      vkDestroyShaderModule(vk->device, shaders_info[0].module, NULL);
      vkDestroyShaderModule(vk->device, shaders_info[1].module, NULL);

      if (shaders_info[2].module)
         vkDestroyShaderModule(vk->device, shaders_info[2].module, NULL);
   }

}

void vk_renderer_destroy(VkDevice device, vk_renderer_t *renderer)
{
   vkDestroyPipeline(device, renderer->pipe, NULL);
   vk_buffer_free(device, &renderer->vbo);
   vk_buffer_free(device, &renderer->ubo);
   vk_buffer_free(device, &renderer->ssbo);
   vk_texture_free(device, &renderer->tex);

   memset(&renderer->vk_renderer_data_start, 0, sizeof(*renderer) - offsetof(vk_renderer_t, vk_renderer_data_start));
}


void vk_renderer_update(VkDevice device, VkCommandBuffer cmd, vk_renderer_t *renderer)
{
   if (renderer->tex.dirty && !renderer->tex.uploaded)
      vk_texture_upload(device, cmd, &renderer->tex);

   int vertex_count = (renderer->vbo.info.range - renderer->vbo.info.offset) / renderer->vertex_stride;

   if (vertex_count < VK_RENDERER_MAX_TEXTURES)
   {
      for (int i = 0; i < vertex_count; i++)
      {
         if (!renderer->textures[i])
            continue;

         if (renderer->textures[i]->ubo.dirty)
            vk_buffer_flush(device, &renderer->textures[i]->ubo);

         if (renderer->textures[i]->dirty && !renderer->textures[i]->flushed)
            vk_texture_flush(device, renderer->textures[i]);

         if (renderer->textures[i]->dirty && !renderer->textures[i]->uploaded)
            vk_texture_upload(device, cmd, renderer->textures[i]);
      }
   }

}


void vk_renderer_exec_simple(VkCommandBuffer cmd, vk_renderer_t *renderer)
{
   if (renderer->vbo.info.range - renderer->vbo.info.offset == 0)
      return;

   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipe);
   {
      uint32_t dynamic_offset = 0;
      VkDescriptorSet desc[] = {renderer->desc, renderer->tex.desc};
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->layout, 0, countof(desc), desc, 1,
                              &dynamic_offset);
   }
   vkCmdBindVertexBuffers(cmd, 0, 1, &renderer->vbo.info.buffer, &renderer->vbo.info.offset);

   int count = (renderer->vbo.info.range - renderer->vbo.info.offset) / renderer->vertex_stride;

   vkCmdDraw(cmd, count, 1, 0, 0);

   renderer->vbo.info.offset = renderer->vbo.info.range;
}

void vk_renderer_exec(VkCommandBuffer cmd, vk_renderer_t *renderer)
{
   if (renderer->vbo.info.range - renderer->vbo.info.offset == 0)
      return;

   vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->pipe);
   uint32_t dynamic_offset = 0;
   vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->layout, 0, 1, &renderer->desc, 1,
                           &dynamic_offset);
   vkCmdBindVertexBuffers(cmd, 0, 1, &renderer->vbo.info.buffer, &renderer->vbo.info.offset);

   int count = (renderer->vbo.info.range - renderer->vbo.info.offset) / renderer->vertex_stride;
   int first_vertex = 0;

   for (uint32_t i = 0; i < count; i++)
   {
      if (i + 1 < count)
      {
         if (i < VK_RENDERER_MAX_TEXTURES)
         {
            if (renderer->textures[i] == renderer->textures[i + 1])
            {
               renderer->textures[i] = NULL;
               continue;
            }
         }
         else
            i = count - 1;
      }


      if (renderer->textures[i])
      {
         vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->layout, 1, 1, &renderer->textures[i]->desc, 0,
                                 NULL);
         renderer->textures[i] = NULL;
      }
      else if (renderer->tex.desc)
         vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, renderer->layout, 1, 1, &renderer->tex.desc, 0, NULL);

      vkCmdDraw(cmd, i + 1 - first_vertex, 1, first_vertex, 0);
      first_vertex = i + 1;
   }

   renderer->vbo.info.offset = renderer->vbo.info.range;
}

void vk_renderer_finish(VkDevice device, vk_renderer_t *renderer)
{
   if (renderer->tex.dirty && !renderer->tex.flushed)
      vk_texture_flush(device, &renderer->tex);

   if (renderer->vbo.dirty)
      vk_buffer_flush(device, &renderer->vbo);

   if (renderer->ubo.dirty)
      vk_buffer_flush(device, &renderer->ubo);

   if (renderer->ssbo.dirty)
      vk_buffer_flush(device, &renderer->ssbo);

   renderer->vbo.info.offset = 0;
   renderer->vbo.info.range = 0;
   renderer->tex.flushed = false;
   renderer->tex.uploaded = false;
}

const char *vk_result_to_str(VkResult res)
{
   switch (res)
   {
#define CASE_TO_STR(res) case res: return #res;
      CASE_TO_STR(VK_SUCCESS);
      CASE_TO_STR(VK_NOT_READY);
      CASE_TO_STR(VK_TIMEOUT);
      CASE_TO_STR(VK_EVENT_SET);
      CASE_TO_STR(VK_EVENT_RESET);
      CASE_TO_STR(VK_INCOMPLETE);
      CASE_TO_STR(VK_ERROR_OUT_OF_HOST_MEMORY);
      CASE_TO_STR(VK_ERROR_OUT_OF_DEVICE_MEMORY);
      CASE_TO_STR(VK_ERROR_INITIALIZATION_FAILED);
      CASE_TO_STR(VK_ERROR_DEVICE_LOST);
      CASE_TO_STR(VK_ERROR_MEMORY_MAP_FAILED);
      CASE_TO_STR(VK_ERROR_LAYER_NOT_PRESENT);
      CASE_TO_STR(VK_ERROR_EXTENSION_NOT_PRESENT);
      CASE_TO_STR(VK_ERROR_FEATURE_NOT_PRESENT);
      CASE_TO_STR(VK_ERROR_INCOMPATIBLE_DRIVER);
      CASE_TO_STR(VK_ERROR_TOO_MANY_OBJECTS);
      CASE_TO_STR(VK_ERROR_FORMAT_NOT_SUPPORTED);
      CASE_TO_STR(VK_ERROR_FRAGMENTED_POOL);
      CASE_TO_STR(VK_ERROR_SURFACE_LOST_KHR);
      CASE_TO_STR(VK_ERROR_NATIVE_WINDOW_IN_USE_KHR);
      CASE_TO_STR(VK_SUBOPTIMAL_KHR);
      CASE_TO_STR(VK_ERROR_OUT_OF_DATE_KHR);
      CASE_TO_STR(VK_ERROR_INCOMPATIBLE_DISPLAY_KHR);
      CASE_TO_STR(VK_ERROR_VALIDATION_FAILED_EXT);
      CASE_TO_STR(VK_ERROR_INVALID_SHADER_NV);
      CASE_TO_STR(VK_ERROR_OUT_OF_POOL_MEMORY_KHR);
      CASE_TO_STR(VK_ERROR_INVALID_EXTERNAL_HANDLE_KHR);
#undef CASE_TO_STR

   default:
      break;
   }

   return "unknown";

}
