#pragma once

#include <stdbool.h>
#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include <vulkan/vulkan.h>
#include "utils/linked_list.h"

#include "video.h"

#define MAX_SWAPCHAIN_IMAGES 8
#define countof(a) (sizeof(a)/ sizeof(*a))

#define VK_CHECK(vk_call) do{VkResult res = vk_call; if (res != VK_SUCCESS) {debug_log("%s:%i:%s:%s --> %s(%i)\n", __FILE__, __LINE__, __FUNCTION__, #vk_call, vk_result_to_str(res), res);fflush(stdout);exit(1);}}while(0)
const char *vk_result_to_str(VkResult res);

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
   bool vsync;
} vk_context_t;

void vk_context_init(vk_context_t *vk);
void vk_context_destroy(vk_context_t *vk);


typedef void(*vk_draw_command_t)(screen_t *screen);

typedef struct vk_draw_command_list_t
{
   vk_draw_command_t draw;
   struct vk_draw_command_list_t *next;
} vk_draw_command_list_t;

typedef struct
{
   VkSurfaceKHR surface;
   screen_t *screen;
   VkDisplayKHR display;
   VkSwapchainKHR swapchain;
   VkRect2D scissor;
   VkViewport viewport;
   uint32_t swapchain_count;
   VkImageView views[MAX_SWAPCHAIN_IMAGES];
   VkFramebuffer framebuffers[MAX_SWAPCHAIN_IMAGES];
   VkCommandBuffer cmd;
   VkFence chain_fence;
   vk_draw_command_list_t *draw_list;
   bool vsync;
} vk_render_target_t;

void vk_render_targets_init(vk_context_t *vk, int count, screen_t *screens, vk_render_target_t *render_targets);
void vk_render_targets_destroy(vk_context_t *vk, int count, vk_render_target_t *render_targets);
void vk_swapchain_init(vk_context_t *vk, vk_render_target_t *render_target);
void vk_swapchain_destroy(vk_context_t *vk, vk_render_target_t *render_target);

static inline void vk_register_draw_command(vk_draw_command_list_t **list, vk_draw_command_t fn)
{
   while (*list)
      list = &(*list)->next;

   *list = malloc(sizeof(*fn));

   (*list)->draw = fn;
   (*list)->next = NULL;
}

static inline void vk_remove_draw_command(vk_draw_command_list_t **list, vk_draw_command_t fn)
{
   /* TODO */
   vk_draw_command_list_t **prev = NULL;

   while (*list)
   {
      if ((*list)->draw == fn)
      {
         vk_draw_command_list_t *tmp = (*list)->next;
         free(*list);

         if (prev)
            (*prev)->next = tmp;

         *list = tmp;

         return;
      }

      prev = list;
      list = &(*list)->next;
   }
}

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
void vk_device_memory_init(VkDevice device, const VkMemoryType *memory_types, const memory_init_info_t *init_info,
   device_memory_t *dst);
void vk_device_memory_free(VkDevice device, device_memory_t *memory);
void vk_device_memory_flush(VkDevice device, const device_memory_t *memory);

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
   VkFilter filter;
   VkDescriptorImageInfo info;
   VkDescriptorSet desc;
   int width;
   int height;
   bool dirty;
   bool is_reference;
   bool flushed;
   bool uploaded;
} vk_texture_t;

void vk_texture_init(vk_context_t* vk, vk_texture_t *dst);
void vk_texture_free(VkDevice device, vk_texture_t *texture);
void vk_texture_upload(VkDevice device, VkCommandBuffer cmd, vk_texture_t *texture);
void vk_texture_flush(VkDevice device, vk_texture_t *texture);

typedef struct
{
   VkDescriptorBufferInfo info;
   device_memory_t mem;
   VkBufferUsageFlags usage;
   bool dirty;
} vk_buffer_t;

void vk_buffer_init(VkDevice device, const VkMemoryType *memory_types, const void *data, vk_buffer_t *dst);
void vk_buffer_flush(VkDevice device, vk_buffer_t *buffer);
void vk_buffer_invalidate(VkDevice device, vk_buffer_t *buffer);
void vk_buffer_free(VkDevice device, vk_buffer_t *buffer);

typedef struct
{
   const uint32_t *code;
   size_t code_size;
} vk_shader_code_t;

typedef struct
{
   struct
   {
      vk_shader_code_t vs;
      vk_shader_code_t ps;
      vk_shader_code_t gs;
   } shaders;

   uint32_t attrib_count;
   const VkVertexInputAttributeDescription *attrib_desc;
   VkPrimitiveTopology topology;
   const VkPipelineColorBlendAttachmentState *color_blend_attachement_state;
} vk_renderer_init_info_t;

typedef struct vk_renderer_t vk_renderer_t;
struct vk_renderer_t
{
   void (*const init)(vk_context_t *vk);
   void (*const destroy)(VkDevice device, vk_renderer_t *renderer);
   void (*const update)(VkDevice device, VkCommandBuffer cmd, vk_renderer_t *renderer);
   void (*const exec)(VkCommandBuffer cmd, vk_renderer_t *renderer);
   void (*const finish)(VkDevice device, vk_renderer_t *renderer);
   vk_texture_t texture;
   vk_buffer_t vbo;
   vk_buffer_t ubo;
   vk_buffer_t ssbo;
   VkDescriptorSet desc;
   VkPipeline pipe;
   VkPipelineLayout layout;
   uint32_t vertex_stride;
   VkCommandBuffer cmd[MAX_SCREENS];
};

#define vk_renderer_data_start texture


void vk_renderer_init(vk_context_t *vk, const vk_renderer_init_info_t *init_info, vk_renderer_t *dst);
void vk_renderer_destroy(VkDevice device, vk_renderer_t *renderer);
void vk_renderer_update(VkDevice device, VkCommandBuffer cmd, vk_renderer_t *renderer);
void vk_renderer_emit(VkCommandBuffer cmd, vk_renderer_t *renderer);
void vk_renderer_finish(VkDevice device, vk_renderer_t *renderer);

static inline void *vk_get_vbo_memory(vk_buffer_t *vbo, VkDeviceSize size)
{
   void *ptr = vbo->mem.u8 + vbo->info.range;
   vbo->info.range += size;
   vbo->dirty = true;
   assert(vbo->info.range <= vbo->mem.size);
   return ptr;
}

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

#define DEBUG_VEC4(v) do{debug_log("%-40s : (%f,%f,%f,%f)\n", #v, v.x, v.y, v.z, v.w); fflush(stdout);}while(0)
