#pragma once

#include "vulkan_common.h"

typedef struct
{
   int count;
   int capacity;
   const char *data[];
} string_list_t;


static inline string_list_t* string_list_create()
{
   string_list_t* dst;
   int capacity = 256;
   dst = (string_list_t*)malloc(sizeof(dst) + capacity * sizeof(*dst->data));
   dst->count = 0;
   dst->capacity = capacity;
   return dst;
}

static inline void string_list_push(string_list_t *dst, const char *string)
{
   if (dst->count == dst->capacity)
   {
      dst->capacity <<= 1;
      dst = realloc(dst, sizeof(*dst) + dst->capacity * sizeof(*dst->data));
   }

   dst->data[dst->count++] = string;
}

typedef struct
{
   int x;
   int y;
   int max_width;
   int max_height;
   struct
   {
      uint8_t r, g, b;
   } color;
   string_list_t* lines;
   bool dry_run;
} font_render_options_t;

void vulkan_font_init(vk_context_t* vk);
void vulkan_font_destroy(VkDevice device);
void vulkan_font_update_assets(VkDevice device, VkCommandBuffer cmd);
void vulkan_font_render(VkCommandBuffer cmd);
void vulkan_font_draw_text(const char *text, const font_render_options_t *options);
