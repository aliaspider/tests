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

typedef struct vk_context_t
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
   struct
   {
      VkDescriptorSetLayout full;
      VkDescriptorSetLayout texture;
   } set_layouts;
   VkPipelineLayout pipeline_layout;
   struct
   {
      VkSampler nearest;
      VkSampler linear;
   } samplers;
   VkRenderPass renderpass;
} vk_context_t;

void vk_context_init(vk_context_t *vk);
void vk_context_destroy(vk_context_t *vk);

typedef struct vk_drawcmd_list_t
{
   draw_command_t draw;
   struct vk_drawcmd_list_t *next;
} vk_drawcmd_list_t;

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
   VkFence chain_fence;
   vk_drawcmd_list_t *draw_list;
   bool vsync;
} vk_render_target_t;

void vk_render_targets_init(vk_context_t *vk, int count, screen_t *screens, vk_render_target_t *render_targets);
void vk_render_targets_destroy(vk_context_t *vk, int count, vk_render_target_t *render_targets);
void vk_swapchain_init(vk_context_t *vk, vk_render_target_t *render_target);
void vk_swapchain_destroy(vk_context_t *vk, vk_render_target_t *render_target);

static inline void vk_register_draw_command(vk_drawcmd_list_t **list, draw_command_t fn)
{
   while (*list)
      list = &(*list)->next;

   *list = malloc(sizeof(**list));

   (*list)->draw = fn;
   (*list)->next = NULL;
}

static inline void vk_remove_draw_command(vk_drawcmd_list_t **list, draw_command_t fn)
{
   /* TODO */
   vk_drawcmd_list_t **prev = NULL;

   while (*list)
   {
      if ((*list)->draw == fn)
      {
         vk_drawcmd_list_t *tmp = (*list)->next;
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
                           device_memory_t *out);
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
   bool ignore_alpha;
   bool dirty;
   bool is_reference;
   bool flushed;
   bool uploaded;
} vk_texture_t;

void vk_texture_init(vk_context_t *vk, vk_texture_t *out);
void vk_texture_free(VkDevice device, vk_texture_t *texture);
void vk_texture_update_descriptor_sets(vk_context_t *vk, vk_texture_t *out);
void vk_texture_upload(VkDevice device, VkCommandBuffer cmd, vk_texture_t *texture);
void vk_texture_flush(VkDevice device, vk_texture_t *texture);

typedef struct
{
   VkDescriptorBufferInfo info;
   device_memory_t mem;
   VkBufferUsageFlags usage;
   bool dirty;
} vk_buffer_t;

void vk_buffer_init(VkDevice device, const VkMemoryType *memory_types, const void *data, vk_buffer_t *out);
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

#define VK_RENDERER_MAX_TEXTURES 64

typedef struct vk_renderer_t vk_renderer_t;
struct vk_renderer_t
{
   void (*const init)(vk_context_t *vk);
   void (*const destroy)(VkDevice device, vk_renderer_t *renderer);
   void (*const update)(VkDevice device, VkCommandBuffer cmd, vk_renderer_t *renderer);
   void (*const exec)(VkCommandBuffer cmd, vk_renderer_t *renderer);
   void (*const finish)(VkDevice device, vk_renderer_t *renderer);
   vk_texture_t tex;
   vk_buffer_t vbo;
   vk_buffer_t ubo;
   vk_buffer_t ssbo;
   VkDescriptorSet desc;
   VkPipeline pipe;
   VkPipelineLayout layout;
   uint32_t vertex_stride;
   vk_texture_t *textures[VK_RENDERER_MAX_TEXTURES + 1];
};

#define vk_renderer_data_start tex

void vk_renderer_init(vk_context_t *vk, const vk_renderer_init_info_t *init_info, vk_renderer_t *out);
void vk_renderer_destroy(VkDevice device, vk_renderer_t *renderer);
void vk_renderer_update(VkDevice device, VkCommandBuffer cmd, vk_renderer_t *renderer);
void vk_renderer_exec(VkCommandBuffer cmd, vk_renderer_t *renderer);
void vk_renderer_exec_simple(VkCommandBuffer cmd, vk_renderer_t *renderer);
void vk_renderer_finish(VkDevice device, vk_renderer_t *renderer);

#define VK_UBO_ALIGNMENT 0x100

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
   float values[4];
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
   float values[4];
} vec4 __attribute__((aligned((sizeof(union vec4)))));

#define DEBUG_VEC4(v) do{debug_log("%-40s : (%f,%f,%f,%f)\n", #v, v.x, v.y, v.z, v.w); fflush(stdout);}while(0)

static inline VkResult VkAllocateCommandBuffer(VkDevice device, VkCommandPool commandPool, VkCommandBuffer* out)
{
   const VkCommandBufferAllocateInfo info =
   {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
      .commandPool = commandPool,
      .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
      .commandBufferCount = 1
   };
   return vkAllocateCommandBuffers(device, &info, out);
}


static inline VkResult VkBeginCommandBuffer(VkCommandBuffer CommandBuffer, VkCommandBufferUsageFlags flags,
      const VkCommandBufferInheritanceInfo *pInheritanceInfo)
{
   const VkCommandBufferBeginInfo info =
   {
      VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
      .flags = flags,
      .pInheritanceInfo = pInheritanceInfo,
   };
   return vkBeginCommandBuffer(CommandBuffer, &info);
}

static inline VkResult VkCreateFence(VkDevice device, bool signaled, VkFence *out)
{
   VkFenceCreateInfo info =
   {
      VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
      .flags = signaled ? VK_FENCE_CREATE_SIGNALED_BIT : 0,
   };
   return vkCreateFence(device, &info, NULL, out);
}
static inline VkResult VkQueueSubmit(VkQueue queue, uint32_t commandBufferCount, const VkCommandBuffer *pCommandBuffers,
                                     VkFence fence)
{
   const VkSubmitInfo info =
   {
      VK_STRUCTURE_TYPE_SUBMIT_INFO,
      .commandBufferCount = commandBufferCount, pCommandBuffers
   };
   return vkQueueSubmit(queue, 1, &info, fence);
}

static inline VkResult VkQueuePresent(VkQueue queue, uint32_t swapchainCount, const VkSwapchainKHR *pSwapchains,
                                      const uint32_t *pImageIndices)
{
   const VkPresentInfoKHR info =
   {
      VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
      .swapchainCount = swapchainCount,
      .pSwapchains = pSwapchains,
      .pImageIndices = pImageIndices
   };
   return vkQueuePresentKHR(queue, &info);
}

static inline void VkCmdBeginRenderPass(VkCommandBuffer commandBuffer, VkRenderPass renderPass, VkFramebuffer framebuffer,
                                 VkRect2D renderArea, const VkClearValue *pClearValue)
{
   const VkRenderPassBeginInfo info =
   {
      VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
      .renderPass = renderPass,
      .framebuffer = framebuffer,
      .renderArea = renderArea,
      .clearValueCount = 1,
      .pClearValues = pClearValue
   };
   vkCmdBeginRenderPass(commandBuffer, &info, VK_SUBPASS_CONTENTS_INLINE);
}

enum
{
   VK_SRC_ALPHA            = VK_BLEND_FACTOR_SRC_ALPHA,
   VK_ONE_MINUS_SRC_ALPHA  = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
   VK_ADD                  = VK_BLEND_OP_ADD,
   VK_COLOR_COMPONENT_ALL  = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
};
