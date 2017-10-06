#pragma once

#include <stdbool.h>
#include <vulkan/vulkan.h>

#define MAX_SWAPCHAIN_IMAGES 8
#define countof(a) (sizeof(a)/ sizeof(*a))
#define VULKAN_CALL(fname, instance, ...)                                      \
  ((PFN_##fname)vkGetInstanceProcAddr(instance, #fname))(instance, __VA_ARGS__)

typedef struct
{
   VkInstance handle;
   VkDebugReportCallbackEXT debug_cb;
}instance_t;
void instance_init(instance_t* dst);
void instance_free(instance_t* context);

typedef struct
{
   VkPhysicalDevice handle;
   union
   {
      struct
      {
         uint32_t        memoryTypeCount;
         VkMemoryType    memoryTypes[VK_MAX_MEMORY_TYPES];
         uint32_t        memoryHeapCount;
         VkMemoryHeap    memoryHeaps[VK_MAX_MEMORY_HEAPS];
      };
      VkPhysicalDeviceMemoryProperties mem;
   };
}physical_device_t;
void physical_device_init(VkInstance instance, physical_device_t* dst);
void physical_device_free(physical_device_t* gpu);

typedef struct
{
   VkDevice handle;
   uint32_t queue_family_index;
   VkQueue queue;
   VkCommandPool cmd_pool;
}device_t;
void device_init(VkPhysicalDevice gpu, device_t* dst);
void device_free(device_t* device);


typedef struct
{
   VkSurfaceKHR handle;
   int width;
   int height;
#ifdef VK_USE_PLATFORM_XLIB_KHR
   Display *display;
   Window   window;
#endif
}surface_t;

typedef struct
{
   VkPhysicalDevice gpu;
   uint32_t queue_family_index;
   int width;
   int height;
}surface_init_info_t;
void surface_init(VkInstance instance, const surface_init_info_t* init_info, surface_t *dst);
void surface_free(VkInstance instance, surface_t *surface);

typedef struct
{
   VkSwapchainKHR handle;
   VkRect2D scissor;
   VkViewport viewport;
   VkRenderPass renderpass;
   uint32_t count;
   VkImageView views[MAX_SWAPCHAIN_IMAGES];
   VkFramebuffer framebuffers[MAX_SWAPCHAIN_IMAGES];
}swapchain_t;

typedef struct
{
   VkSurfaceKHR surface;
   int width;
   int height;
   VkPresentModeKHR present_mode;
}swapchain_init_info_t;
void swapchain_init(VkDevice device, const swapchain_init_info_t* init_info, swapchain_t *dst);
void swapchain_free(VkDevice device, swapchain_t *chain);

typedef struct
{
   VkDeviceMemory handle;
   VkMemoryPropertyFlags flags;
   VkDeviceSize size;
   VkDeviceSize alignment;
   union
   {
      void* ptr;
      uint8_t* u8;
   };
}device_memory_t;

typedef struct
{
   VkMemoryPropertyFlags req_flags;
   VkBuffer buffer;
   VkImage image;
}memory_init_info_t;
void device_memory_init(VkDevice device, const VkMemoryType* memory_types, const memory_init_info_t* init_info, device_memory_t* dst);
void device_memory_free(VkDevice device, device_memory_t* memory);
void device_memory_flush(VkDevice device, const device_memory_t* memory);

typedef struct
{
   struct
   {
      device_memory_t mem;
      VkImage image;
      VkSubresourceLayout mem_layout;
      VkImageLayout layout;
   }staging;
   device_memory_t mem;
   VkImage image;
   VkSubresourceLayout mem_layout;
   VkImageLayout layout;
   VkImageView view;
   VkSampler sampler;
   int width;
   int height;
   bool dirty;
}texture_t;

typedef struct
{
   uint32_t queue_family_index;
   int width;
   int height;
}texture_init_info_t;
void texture_init(VkDevice device, const VkMemoryType* memory_types, const texture_init_info_t *init_info, texture_t* dst);
void texture_free(VkDevice device, texture_t* texture);
void texture_update(VkCommandBuffer cmd, texture_t* texture);

typedef struct
{
   device_memory_t mem;
   VkBuffer handle;
   VkDeviceSize size;
}buffer_t;

typedef struct
{
   VkBufferUsageFlags usage;
   uint32_t size;
   const void* data;
}buffer_init_info_t;
void buffer_init(VkDevice device, const VkMemoryType* memory_types, const buffer_init_info_t* init_info, buffer_t *dst);
void buffer_free(VkDevice device, buffer_t *buffer);

typedef struct
{
   VkDescriptorPool pool;
   VkDescriptorSet set;
   VkDescriptorSetLayout set_layout;
}descriptor_t;

typedef struct
{
   VkBuffer ubo_buffer;
   VkDeviceSize ubo_range;
   VkSampler sampler;
   VkImageView image_view;
}descriptors_init_info_t;
void descriptors_init(VkDevice device, const descriptors_init_info_t* init_info, descriptor_t* dst);
void descriptors_free(VkDevice device, descriptor_t* descriptor);

typedef struct
{
   VkPipeline handle;
   VkPipelineLayout layout;
}pipeline_t;

typedef struct
{
   VkShaderModule vertex_shader;
   VkShaderModule fragment_shader;
   int vertex_size;
   int attrib_count;
   const VkVertexInputAttributeDescription* attrib_desc;
   VkDescriptorSetLayout set_layout;
   const VkRect2D* scissor;
   const VkViewport* viewport;
   VkRenderPass renderpass;
}pipeline_init_info_t;
void pipeline_init(VkDevice device, const pipeline_init_info_t* init_info, pipeline_t* dst);
void pipeline_free(VkDevice device, pipeline_t* pipe);
