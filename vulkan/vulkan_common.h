#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <vulkan/vulkan.h>

#include "video.h"

#define MAX_SWAPCHAIN_IMAGES 8
#define countof(a) (sizeof(a)/ sizeof(*a))

typedef struct
{
   VkDeviceMemory handle;
   VkMemoryPropertyFlags flags;
   VkDeviceSize size;
   VkDeviceSize alignment;
   VkSubresourceLayout layout;
   union
   {
      void *ptr;
      uint8_t *u8;
   };
} device_memory_t;

typedef struct
{
   VkMemoryPropertyFlags req_flags;
   VkBuffer buffer;
   VkImage image;
} memory_init_info_t;
void device_memory_init(VkDevice device, const VkMemoryType *memory_types, const memory_init_info_t *init_info,
   device_memory_t *dst);
void device_memory_free(VkDevice device, device_memory_t *memory);
void device_memory_flush(VkDevice device, const device_memory_t *memory);

typedef struct
{
   struct
   {
      device_memory_t mem;
      VkImage image;
      VkFormat format;
      VkImageLayout layout;
   } staging;
   device_memory_t mem;
   VkImage image;
   VkFormat format;
   VkDescriptorImageInfo info;
   int width;
   int height;
   bool dirty;
   bool is_reference;
} vk_texture_t;

void vk_texture_init(VkDevice device, const VkMemoryType *memory_types, uint32_t queue_family_index, vk_texture_t *dst);
void texture_free(VkDevice device, vk_texture_t *texture);
void texture_update(VkDevice device, VkCommandBuffer cmd, vk_texture_t *texture);

typedef struct
{
   union
   {
      struct VkDescriptorBufferInfo;
      VkDescriptorBufferInfo info;
   };
   device_memory_t mem;
   VkBufferUsageFlags usage;
   bool dirty;
} vk_buffer_t;

void vk_buffer_init(VkDevice device, const VkMemoryType *memory_types, const void *data, vk_buffer_t *dst);
void buffer_flush(VkDevice device, vk_buffer_t *buffer);
void buffer_free(VkDevice device, vk_buffer_t *buffer);

typedef struct
{
   struct
   {
      float x, y, z, w;
   } position;
   struct
   {
      float u, v;
   } texcoord;
   struct
   {
      float r, g, b, a;
   } color;
} vertex_t;

static inline const char *VkResult_to_str(VkResult res)
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

#define VK_CHECK(vk_call) do{VkResult res = vk_call; if (res != VK_SUCCESS) {printf("%s:%i:%s:%s --> %s(%i)\n", __FILE__, __LINE__, __FUNCTION__, #vk_call, VkResult_to_str(res), res);fflush(stdout);}}while(0)

void vk_init_instance_pfn(VkInstance instance);
void vk_init_device_pfn(VkDevice device);
void vk_get_instance_props(void);
void vk_get_gpu_props(VkPhysicalDevice gpu);
void vk_get_surface_props(VkPhysicalDevice gpu, uint32_t queue_family_index, VkSurfaceKHR surface);

uint32_t vk_get_queue_family_index(VkPhysicalDevice gpu, VkQueueFlags required_flags);


typedef struct
{
   VkInstance instance;
   VkDebugReportCallbackEXT debug_cb;
   VkPhysicalDevice gpu;
   union
   {
      struct
      {
         uint32_t memoryTypeCount;
         VkMemoryType memoryTypes[VK_MAX_MEMORY_TYPES];
      };
      VkPhysicalDeviceMemoryProperties mem;
   };
   VkDevice device;
   uint32_t queue_family_index;
   VkQueue queue;
   VkFence queue_fence;
   struct
   {
      VkCommandPool cmd;
      VkDescriptorPool desc;
   } pools;
   VkDescriptorSetLayout descriptor_set_layout;
   VkPipelineLayout pipeline_layout;
   struct
   {
      VkSampler nearest;
      VkSampler linear;
   } samplers;
   VkRenderPass renderpass;
} vk_context_t;

typedef struct
{
   VkSurfaceKHR surface;
   screen_t* screen;
   VkDisplayKHR display;
   VkSwapchainKHR swapchain;
   VkRect2D scissor;
   VkViewport viewport;
   uint32_t swapchain_count;
   VkImageView views[MAX_SWAPCHAIN_IMAGES];
   VkFramebuffer framebuffers[MAX_SWAPCHAIN_IMAGES];
   VkCommandBuffer cmd;
   VkFence chain_fence;
} vk_render_context_t;

typedef struct
{
   vk_texture_t texture;
   vk_buffer_t vbo;
   vk_buffer_t ubo;
   vk_buffer_t ssbo;
   VkDescriptorSet desc;
   VkPipeline handle;
   VkPipelineLayout layout;
}vk_pipeline_t;

typedef struct
{
   const uint32_t* code;
   size_t code_size;
}vk_shader_code_t;

typedef struct
{
   struct
   {
      vk_shader_code_t vs;
      vk_shader_code_t ps;
      vk_shader_code_t gs;
   }shaders;

   uint32_t vertex_stride;
   uint32_t attrib_count;
   const VkVertexInputAttributeDescription* attrib_desc;
   VkPrimitiveTopology topology;
   const VkPipelineColorBlendAttachmentState* color_blend_attachement_state;
}vk_pipeline_init_info_t;

void vk_pipeline_init(vk_context_t *vk, const vk_pipeline_init_info_t *init_info, vk_pipeline_t *dst);
void vk_pipeline_destroy(VkDevice device, vk_pipeline_t *render);

typedef union
{
   struct
   {
      float r;
      float g;
   };
   struct
   {
      float x;
      float y;
   };
   struct
   {
      float width;
      float height;
   };
} vec2;

typedef union vec4
{
   struct
   {
      float r;
      float g;
      float b;
      float a;
   };
   struct
   {
      float x;
      float y;
      union
      {
         struct
         {
            float z;
            float w;
         };
         struct
         {
            float width;
            float height;
         };
      };
   };
} vec4 __attribute__((aligned((sizeof(union vec4)))));
