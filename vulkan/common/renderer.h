#pragma once

#include <assert.h>

#include "common.h"
#include "texture.h"
#include "buffer.h"
#include "video.h"

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
   void (*const destroy)(vk_renderer_t *renderer, VkDevice device);
   void (*const begin)(vk_renderer_t *renderer, screen_t* screen);
   VkCommandBuffer (*const finish)(vk_renderer_t *renderer);
   vk_texture_t default_texture;
   vk_buffer_t vbo;
   vk_buffer_t ubo;
   vk_buffer_t ssbo;
   struct
   {
      VkDescriptorSet main;
      VkDescriptorSet texture;
   }desc;
   VkPipeline pipe;
   VkPipelineLayout pipeline_layout;
   VkRenderPass renderpass;
   uint32_t vertex_stride;
   uint32_t first_vertex;
   VkCommandBuffer cmds[MAX_SCREENS];
   VkCommandBuffer cmd;
};

#define vk_renderer_data_start default_texture

void vk_renderer_init(vk_context_t *vk, const vk_renderer_init_info_t *init_info, vk_renderer_t *out);
void vk_renderer_destroy(vk_renderer_t *renderer, VkDevice device);
void vk_renderer_begin(vk_renderer_t *renderer, screen_t* screen);
void vk_renderer_bind_texture(vk_renderer_t *renderer, vk_texture_t *texture);
VkCommandBuffer vk_renderer_finish(vk_renderer_t *renderer);

static inline void *vk_get_vbo_memory(vk_buffer_t *vbo, VkDeviceSize size)
{
   void *ptr = vbo->mem.u8 + vbo->info.offset + vbo->info.range;
   vbo->info.range += size;
   vbo->dirty = true;
   assert(vbo->info.range <= vbo->mem.size);
   return ptr;
}
