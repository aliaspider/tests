
#include <string.h>
#include <assert.h>

#include "video.h"
#include "common.h"
#include "vulkan_common.h"
#include "frame.h"
#include "font.h"

video_t video;

typedef struct
{
   VkInstance instance;
   VkDebugReportCallbackEXT debug_cb;
   VkPhysicalDevice gpu;
   union
   {
      struct
      {
         uint32_t        memoryTypeCount;
         VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES];
      };
      VkPhysicalDeviceMemoryProperties mem;
   };
   VkDevice device;
   uint32_t queue_family_index;
   VkQueue queue;
   VkCommandPool cmd_pool;
   VkSurfaceKHR surface;
   int surface_width;
   int surface_height;
   VkDisplayKHR display;
} vk_context_t;

typedef struct
{
   VkSwapchainKHR swapchain;
   VkRect2D scissor;
   VkViewport viewport;
   VkRenderPass renderpass;
   uint32_t swapchain_count;
   VkImageView views[MAX_SWAPCHAIN_IMAGES];
   VkFramebuffer framebuffers[MAX_SWAPCHAIN_IMAGES];
   VkDescriptorPool descriptor_pool;
   VkDescriptorSetLayout descriptor_set_layout;
   VkCommandBuffer cmd;
   VkFence queue_fence;
   VkFence chain_fence;
} vk_render_context_t;

static vk_context_t vk;
static vk_render_context_t vk_render;

VkBool32 vulkan_debug_report_callback(VkDebugReportFlagsEXT flags,
                                      VkDebugReportObjectTypeEXT objectType,
                                      uint64_t object, size_t location,
                                      int32_t messageCode,
                                      const char *pLayerPrefix,
                                      const char *pMessage, void *pUserData)
{
   static const char *debugFlags_str[] = {"INFORMATION", "WARNING", "PERFORMANCE", "ERROR", "DEBUG"};

   int i;

   for (i = 0; i < countof(debugFlags_str); i++)
      if (flags & (1 << i))
         break;

   printf("[%-14s - %-11s] %s\n", pLayerPrefix, debugFlags_str[i],
          pMessage);

#ifdef HAVE_X11
   XAutoRepeatOn(video.screen.display);
#endif

   assert((flags & VK_DEBUG_REPORT_ERROR_BIT_EXT) == 0);
   return VK_FALSE;
}

void video_init()
{
   debug_log("video init\n");

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
         int l, e;
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

            for (e = 0; e < iexprop_count; e++)
               printf("\t%s\n", iexprops[e].extensionName);
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
      VK_CHECK(vkCreateInstance(&info, NULL, &vk.instance));
   }

   {
      VkDebugReportCallbackCreateInfoEXT info =
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
      vkCreateDebugReportCallbackEXT(vk.instance, &info, NULL, &vk.debug_cb);
   }

   pvkGetPhysicalDeviceSurfaceCapabilities2EXT = (PFN_vkGetPhysicalDeviceSurfaceCapabilities2EXT)
         vkGetInstanceProcAddr(vk.instance, "vkGetPhysicalDeviceSurfaceCapabilities2EXT");
#ifdef VK_USE_PLATFORM_XLIB_XRANDR_EXT
   pvkReleaseDisplayEXT = (PFN_vkReleaseDisplayEXT)vkGetInstanceProcAddr(vk.instance,
                          "vkReleaseDisplayEXT");
   pvkAcquireXlibDisplayEXT = (PFN_vkAcquireXlibDisplayEXT)vkGetInstanceProcAddr(vk.instance,
                              "vkAcquireXlibDisplayEXT");
   pvkGetRandROutputDisplayEXT = (PFN_vkGetRandROutputDisplayEXT)vkGetInstanceProcAddr(vk.instance,
                                 "vkGetRandROutputDisplayEXT");
#endif


   {
      uint32_t one = 1;
      vkEnumeratePhysicalDevices(vk.instance, &one, &vk.gpu);
   }

   vkGetPhysicalDeviceMemoryProperties(vk.gpu, &vk.mem);

#if 1
   VkPhysicalDeviceProperties gpu_props;
   vkGetPhysicalDeviceProperties(vk.gpu, &gpu_props);
   uint32_t deviceExtensionPropertiesCount;
   vkEnumerateDeviceExtensionProperties(vk.gpu, NULL, &deviceExtensionPropertiesCount, NULL);
   VkExtensionProperties pDeviceExtensionProperties[deviceExtensionPropertiesCount];
   vkEnumerateDeviceExtensionProperties(vk.gpu, NULL, &deviceExtensionPropertiesCount,
                                        pDeviceExtensionProperties);
   int e;

   for (e = 0; e < deviceExtensionPropertiesCount; e++)
      printf("\t%s\n", pDeviceExtensionProperties[e].extensionName);

#endif

   {
      uint32_t queueFamilyPropertyCount;
      vkGetPhysicalDeviceQueueFamilyProperties(vk.gpu, &queueFamilyPropertyCount, NULL);
      VkQueueFamilyProperties pQueueFamilyProperties[queueFamilyPropertyCount];
      vkGetPhysicalDeviceQueueFamilyProperties(vk.gpu, &queueFamilyPropertyCount, pQueueFamilyProperties);

      vk.queue_family_index = 0;

      int i;

      for (i = 0; i < queueFamilyPropertyCount; i++)
      {
         if ((pQueueFamilyProperties[i].queueFlags & (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT)) ==
               (VK_QUEUE_GRAPHICS_BIT | VK_QUEUE_TRANSFER_BIT))
         {
            vk.queue_family_index = i;
            break;
         }
      }
   }
   {
      const float one = 1.0;
      const char *device_ext[] =
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
         .queueFamilyIndex = vk.queue_family_index,
         .queueCount = 1,
         .pQueuePriorities = &one
      };

      VkPhysicalDeviceFeatures enabledFeatures =
      {
         .samplerAnisotropy = VK_TRUE,
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
      vkCreateDevice(vk.gpu, &info, NULL, &vk.device);
   }

   /* get a device queue */
   vkGetDeviceQueue(vk.device, vk.queue_family_index, 0, &vk.queue);

   /* create command buffer pool */
   {
      VkCommandPoolCreateInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
         .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT | VK_COMMAND_POOL_CREATE_RESET_COMMAND_BUFFER_BIT,
         .queueFamilyIndex = vk.queue_family_index
      };
      vkCreateCommandPool(vk.device, &info, NULL, &vk.cmd_pool);
   }

   video.screen.width = 640;
   video.screen.height = 480;
#ifdef VK_USE_PLATFORM_XLIB_KHR
   video.screen.display = XOpenDisplay(NULL);
   video.screen.window  = XCreateSimpleWindow(video.screen.display,
                          DefaultRootWindow(video.screen.display), 0, 0, video.screen.width, video.screen.height, 0, 0, 0);
   XStoreName(video.screen.display, video.screen.window, "Vulkan Test");
   XSelectInput(video.screen.display, video.screen.window,
                ExposureMask | FocusChangeMask | KeyPressMask | KeyReleaseMask);
   XMapWindow(video.screen.display, video.screen.window);
#endif

   /* init display and window */
#ifdef VK_USE_PLATFORM_XLIB_KHR

   /* init surface */
   {
      VkXlibSurfaceCreateInfoKHR info =
      {
         VK_STRUCTURE_TYPE_XLIB_SURFACE_CREATE_INFO_KHR,
         .dpy = video.screen.display,
         .window = video.screen.window
      };
      vkCreateXlibSurfaceKHR(vk.instance, &info, NULL, &vk.surface);
   }
#elif defined(VK_USE_PLATFORM_WIN32_KHR)

#else
#error platform not supported
#endif
#if 0
#ifdef __linux__
   {
      {
         uint32_t displayProperties_count;
         vkGetPhysicalDeviceDisplayPropertiesKHR(vk.gpu, &displayProperties_count, NULL);
         VkDisplayPropertiesKHR displayProperties[displayProperties_count];
         vkGetPhysicalDeviceDisplayPropertiesKHR(vk.gpu, &displayProperties_count, displayProperties);
         dst->display = displayProperties[0].display;
         int i;

         for (i = 0; i < displayProperties_count; i++)
            printf("0x%08"PRIXPTR" : %s\n", (uintptr_t)displayProperties[i].display, displayProperties[i].displayName);

            }
            vkGetPhysicalDeviceDisplayPlanePropertiesKHR(vk.gpu, &displayProperties_count, NULL);
            VkDisplayPlanePropertiesKHR displayPlaneProperties[displayProperties_count];
            vkGetPhysicalDeviceDisplayPlanePropertiesKHR(vk.gpu, &displayProperties_count, displayPlaneProperties);
            for (i = 0; i < displayProperties_count; i++)
            {
               printf("0x%08"PRIXPTR" : %i\n", (uintptr_t)displayPlaneProperties[i].currentDisplay, displayPlaneProperties[i].currentStackIndex);

            }
         }
   //      uint32_t displayCount = 4;
   //      VK_CHECK(vkGetDisplayPlaneSupportedDisplaysKHR(vk.gpu, 1, &displayCount, &dst->display));


   //      VK_CHECK(vkGetRandROutputDisplayEXT(vk.gpu, init_info->display, 0x27e, &dst->display));
   //      VK_CHECK(vkAcquireXlibDisplayEXT(vk.gpu, init_info->display, dst->display));
   //      VK_CHECK(vkReleaseDisplayEXT(vk.gpu, dst->display));
   //      exit(0);


         uint32_t displayModeCount;
         VK_CHECK(vkGetDisplayModePropertiesKHR(vk.gpu, dst->display, &displayModeCount, NULL));
         VkDisplayModePropertiesKHR displayModeProperties[displayModeCount];
         VK_CHECK(vkGetDisplayModePropertiesKHR(vk.gpu, dst->display, &displayModeCount, displayModeProperties));
         printf("displayModeProperties.parameters.refreshRate : %u\n", displayModeProperties[0].parameters.refreshRate);
         printf("displayModeProperties.visibleRegion.width : %u\n", displayModeProperties[0].parameters.visibleRegion.width);
         printf("displayModeProperties.visibleRegion.height : %u\n", displayModeProperties[0].parameters.visibleRegion.height);
      }
#endif
#endif
   VkSurfaceCapabilitiesKHR surfaceCapabilities;
   vkGetPhysicalDeviceSurfaceCapabilitiesKHR(vk.gpu, vk.surface, &surfaceCapabilities);
#if 0
   VkSurfaceCapabilities2EXT surfaceCapabilities2 = {VK_STRUCTURE_TYPE_SURFACE_CAPABILITIES_2_EXT};
   vkGetPhysicalDeviceSurfaceCapabilities2EXT(vk.gpu, dst->handle, &surfaceCapabilities2);
   printf("surfaceCapabilities2.supportedSurfaceCounters : %i\n", surfaceCapabilities2.supportedSurfaceCounters);
#endif
   VkBool32 physicalDeviceSurfaceSupport;
   vkGetPhysicalDeviceSurfaceSupportKHR(vk.gpu, vk.queue_family_index, vk.surface, &physicalDeviceSurfaceSupport);

   uint32_t surfaceFormatcount;
   vkGetPhysicalDeviceSurfaceFormatsKHR(vk.gpu, vk.surface, &surfaceFormatcount, NULL);
   VkSurfaceFormatKHR surfaceFormats[surfaceFormatcount];
   vkGetPhysicalDeviceSurfaceFormatsKHR(vk.gpu, vk.surface, &surfaceFormatcount, surfaceFormats);

   uint32_t presentModeCount;
   vkGetPhysicalDeviceSurfacePresentModesKHR(vk.gpu, vk.surface, &presentModeCount, NULL);
   VkPresentModeKHR presentModes[presentModeCount];
       vkGetPhysicalDeviceSurfacePresentModesKHR(vk.gpu, vk.surface, &presentModeCount, presentModes);

   vk.surface_width = video.screen.width;
   vk.surface_height = video.screen.height;

   VkSwapchainCounterCreateInfoEXT swapchainCounterCreateInfo =
   {
      VK_STRUCTURE_TYPE_SWAPCHAIN_COUNTER_CREATE_INFO_EXT,
      .surfaceCounters = VK_SURFACE_COUNTER_VBLANK_EXT
   };

   VkSwapchainCreateInfoKHR swapchainCreateInfo =
   {
      VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
      .pNext = &swapchainCounterCreateInfo,
      .surface = vk.surface,
      .minImageCount = 2,
      .imageFormat = VK_FORMAT_B8G8R8A8_UNORM,
      .imageColorSpace = VK_COLOR_SPACE_SRGB_NONLINEAR_KHR,
      .imageExtent.width = vk.surface_width,
      .imageExtent.height = vk.surface_height,
      .imageArrayLayers = 1,
      .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
      .imageSharingMode = VK_SHARING_MODE_EXCLUSIVE,
      .preTransform = VK_SURFACE_TRANSFORM_IDENTITY_BIT_KHR,
      .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
      .presentMode = VK_PRESENT_MODE_FIFO_KHR,
//         .present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR
      .clipped = VK_TRUE
   };
   VK_CHECK(vkCreateSwapchainKHR(vk.device, &swapchainCreateInfo, NULL, &vk_render.swapchain));

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

   vkCreateRenderPass(vk.device, &renderPassCreateInfo, NULL, &vk_render.renderpass);

   /* init image views and framebuffers */
   vkGetSwapchainImagesKHR(vk.device, vk_render.swapchain, &vk_render.swapchain_count, NULL);
   if(vk_render.swapchain_count > MAX_SWAPCHAIN_IMAGES)
      vk_render.swapchain_count = MAX_SWAPCHAIN_IMAGES;

   VkImage swapchainImages[vk_render.swapchain_count];
   vkGetSwapchainImagesKHR(vk.device, vk_render.swapchain, &vk_render.swapchain_count, swapchainImages);


   int i;
   for (i = 0; i < vk_render.swapchain_count; i++)
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
         vkCreateImageView(vk.device, &info, NULL, &vk_render.views[i]);
      }

      {
         VkFramebufferCreateInfo info =
         {
            VK_STRUCTURE_TYPE_FRAMEBUFFER_CREATE_INFO,
            .renderPass = vk_render.renderpass,
            .attachmentCount = 1, &vk_render.views[i],
            .width = vk.surface_width,
            .height = vk.surface_height,
            .layers = 1
         };
         vkCreateFramebuffer(vk.device, &info, NULL, &vk_render.framebuffers[i]);
      }
   }

   vk_render.viewport.x = 0.0f;
   vk_render.viewport.y = 0.0f;
   vk_render.viewport.width = vk.surface_width;
   vk_render.viewport.height = vk.surface_height;
   vk_render.viewport.minDepth = -1.0f;
   vk_render.viewport.maxDepth =  1.0f;

   vk_render.scissor.offset.x = 0.0f;
   vk_render.scissor.offset.y = 0.0f;
   vk_render.scissor.extent.width = vk.surface_width;
   vk_render.scissor.extent.height = vk.surface_height;



   {
      const VkDescriptorPoolSize sizes[] =
      {
         {VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER, 2},
         {VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, 2}
      };

      const VkDescriptorPoolCreateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
         .maxSets = 2,
         .poolSizeCount = countof(sizes), sizes
      };
      vkCreateDescriptorPool(vk.device, &info, NULL, &vk_render.descriptor_pool);
   }

   {
      const VkDescriptorSetLayoutBinding bindings[] =
      {
         {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_VERTEX_BIT,
         },
         {
            .binding = 1,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = 1,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT,
   //            .pImmutableSamplers = &vk.texture_sampler
         }
      };

      const VkDescriptorSetLayoutCreateInfo info [] =
      {
         {
            VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
            .bindingCount = countof(bindings), bindings

         }
      };
      vkCreateDescriptorSetLayout(vk.device, &info[0], NULL, &vk_render.descriptor_set_layout);
   }



   {
      const VkCommandBufferAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
         .commandPool = vk.cmd_pool,
         .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
         .commandBufferCount = 1
      };
      vkAllocateCommandBuffers(vk.device, &info, &vk_render.cmd);
   }

   {
      VkFenceCreateInfo info =
      {
         VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
         .flags = VK_FENCE_CREATE_SIGNALED_BIT
      };
      vkCreateFence(vk.device, &info, NULL, &vk_render.chain_fence);
      vkCreateFence(vk.device, &info, NULL, &vk_render.queue_fence);
   }

//   {
//      VkDisplayEventInfoEXT displayEventInfo =
//      {
//         VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT,
//         .displayEvent = VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT
//      };
//      VK_CHECK(vkRegisterDisplayEventEXT(vk.device, surface.display, &displayEventInfo, NULL, &display_fence));
//   }

   //   uniforms_t *uniforms = ubo.mem.ptr;
   //   uniforms->center.x = 0.0f;
   //   uniforms->center.y = 0.0f;
   //   uniforms->image.width  = tex.width;
   //   uniforms->image.height = tex.height;
   //   uniforms->screen.width  = chain.viewport.width;
   //   uniforms->screen.height = chain.viewport.height;
   //   uniforms->angle = 0.0f;

   vulkan_font_init(vk.device, vk.queue_family_index, vk.memoryTypes, vk_render.descriptor_pool, vk_render.descriptor_set_layout, &vk_render.scissor, &vk_render.viewport, vk_render.renderpass);

}

void video_frame_update()
{
   uint32_t image_index;
   vkWaitForFences(vk.device, 1, &vk_render.chain_fence, VK_TRUE, -1);
   vkResetFences(vk.device, 1, &vk_render.chain_fence);
   vkAcquireNextImageKHR(vk.device, vk_render.swapchain, UINT64_MAX, NULL, vk_render.chain_fence, &image_index);

   vkWaitForFences(vk.device, 1, &vk_render.queue_fence, VK_TRUE, -1);
   vkResetFences(vk.device, 1, &vk_render.queue_fence);

//   vkWaitForFences(vk.device, 1, &display_fence, VK_TRUE, -1);
//   vkResetFences(vk.device, 1, &display_fence);

   {
      const VkCommandBufferBeginInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
      };
      vkBeginCommandBuffer(vk_render.cmd, &info);
   }

   vulkan_frame_update(vk.device, vk_render.cmd);
   vulkan_font_update_assets(vk_render.cmd);

   /* renderpass */
   {
      {
         const VkClearValue clearValue = {{{0.0f, 0.1f, 1.0f, 0.0f}}};
         const VkRenderPassBeginInfo info =
         {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = vk_render.renderpass,
            .framebuffer = vk_render.framebuffers[image_index],
            .renderArea = vk_render.scissor,
            .clearValueCount = 1,
            .pClearValues = &clearValue
         };
         vkCmdBeginRenderPass(vk_render.cmd, &info, VK_SUBPASS_CONTENTS_INLINE);
      }

      vulkan_frame_render(vk_render.cmd);
//      vulkan_font_render(cmd);

      vkCmdEndRenderPass(vk_render.cmd);
   }

   vkEndCommandBuffer(vk_render.cmd);

   {
      const VkSubmitInfo info =
      {
         VK_STRUCTURE_TYPE_SUBMIT_INFO,
         .commandBufferCount = 1,
         .pCommandBuffers = &vk_render.cmd
      };
      vkQueueSubmit(vk.queue, 1, &info, vk_render.queue_fence);
   }

   {
      const VkPresentInfoKHR info =
      {
         VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
         .swapchainCount = 1,
         .pSwapchains = &vk_render.swapchain,
         .pImageIndices = &image_index
      };
      vkQueuePresentKHR(vk.queue, &info);
   }
#if 0
   {
      uint64_t vblank_counter = 0;
      VK_CHECK(vkGetSwapchainCounterEXT(vk.device, chain.handle, VK_SURFACE_COUNTER_VBLANK_EXT, &vblank_counter));
      printf("vblank_counter : %"PRId64"\n", vblank_counter);
   }
#endif
}

void video_destroy()
{

   vkWaitForFences(vk.device, 1, &vk_render.queue_fence, VK_TRUE, UINT64_MAX);
   vkDestroyFence(vk.device, vk_render.queue_fence, NULL);

   vkWaitForFences(vk.device, 1, &vk_render.chain_fence, VK_TRUE, UINT64_MAX);
   vkDestroyFence(vk.device, vk_render.chain_fence, NULL);

//   vkWaitForFences(vk.device, 1, &display_fence, VK_TRUE, UINT64_MAX);
//   vkDestroyFence(vk.device, display_fence, NULL);

   vulkan_font_destroy(vk.device, vk_render.descriptor_pool);
   vulkan_frame_destroy(vk.device);

   vkDestroyDescriptorPool(vk.device, vk_render.descriptor_pool, NULL);
   vkDestroyDescriptorSetLayout(vk.device, vk_render.descriptor_set_layout, NULL);

   //   buffer_free(vk.device, &ubo);
   int i;
   for (i = 0; i < vk_render.swapchain_count; i++)
   {
      vkDestroyImageView(vk.device, vk_render.views[i], NULL);
      vkDestroyFramebuffer(vk.device, vk_render.framebuffers[i], NULL);
   }
   vkDestroySwapchainKHR(vk.device, vk_render.swapchain, NULL);
   vkDestroyRenderPass(vk.device, vk_render.renderpass, NULL);

   vkDestroySurfaceKHR(vk.instance, vk.surface, NULL);
   vkDestroyCommandPool(vk.device, vk.cmd_pool, NULL);
   vkDestroyDevice(vk.device, NULL);
   vkDestroyDebugReportCallbackEXT(vk.instance, vk.debug_cb, NULL);
   vkDestroyInstance(vk.instance, NULL);

   memset(&vk, 0, sizeof(vk));
   memset(&vk_render, 0, sizeof(vk_render));

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
   vulkan_frame_init(vk.device, vk.queue_family_index, vk.memoryTypes, vk_render.descriptor_pool, vk_render.descriptor_set_layout,
                     width, height,
                     format == screen_format_RGB565 ? VK_FORMAT_R5G6B5_UNORM_PACK16 :
                                                      format == screen_format_ARGB5551 ? VK_FORMAT_R5G5B5A1_UNORM_PACK16 :
                                                                                         VK_FORMAT_R8G8B8A8_SRGB,
                     &vk_render.scissor, &vk_render.viewport, vk_render.renderpass);


}

const video_t video_vulkan =
{
   .init = video_init,
   .frame_init = video_frame_init,
   .frame_update = video_frame_update,
   .destroy = video_destroy
};
