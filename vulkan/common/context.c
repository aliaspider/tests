
#include <string.h>
#include <assert.h>

#include "context.h"
#include "stubs.h"
#include "inlines.h"

#ifdef DEBUG
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
         || messageCode == 68
//      || messageCode == 38
//      || messageCode == 438304791
//      || messageCode == 9
      )
      return VK_FALSE;

   int i;

   for (i = 0; i < countof(debugFlags_str); i++)
      if (flags & (1 << i))
         break;

   debug_log("[%i-%-14s - %-11s] %s\n", messageCode, pLayerPrefix, debugFlags_str[i], pMessage);
   fflush(stdout);

   assert((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) == 0);
   return VK_FALSE;
}
#endif

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

static inline uint32_t vk_get_queue_family_index(VkPhysicalDevice gpu, VkQueueFlags required_flags)
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

void vk_context_init(vk_context_t *vk)
{

   {
      static const char *layers[] =
      {
#ifdef DEBUG
         "VK_LAYER_LUNARG_standard_validation",
         "VK_LAYER_GOOGLE_unique_objects",
         "VK_LAYER_LUNARG_core_validation",
         "VK_LAYER_LUNARG_object_tracker",
         "VK_LAYER_LUNARG_parameter_validation",
         "VK_LAYER_GOOGLE_threading"
#endif
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
#ifdef DEBUG
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
#endif
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
            .descriptorType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER_DYNAMIC,
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
      const VkDescriptorSetLayoutBinding bindings[] =
      {
         {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 2,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
            .pImmutableSamplers = (VkSampler *) &vk->samplers
         },
         {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_GEOMETRY_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
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
      vkCreateDescriptorSetLayout(vk->device, &info, NULL, &vk->set_layouts.renderer);
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
            .size = 2 * sizeof(float)
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
   vkDestroyDescriptorSetLayout(vk->device, vk->set_layouts.renderer, NULL);
   vkDestroyDescriptorSetLayout(vk->device, vk->set_layouts.texture, NULL);
   vkDestroyPipelineLayout(vk->device, vk->pipeline_layout, NULL);
   vkDestroyRenderPass(vk->device, vk->renderpass, NULL);
   vkDestroyCommandPool(vk->device, vk->pools.cmd, NULL);
   vkDestroyDevice(vk->device, NULL);
#ifdef DEBUG
   vkDestroyDebugReportCallbackEXT(vk->instance, vk->debug_cb, NULL);
#endif
   vkDestroyInstance(vk->instance, NULL);
   memset(vk, 0, sizeof(*vk));
}

