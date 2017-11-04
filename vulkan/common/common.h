#pragma once

#include <vulkan/vulkan.h>

#include <stdio.h>
#include <stdlib.h>

#include "utils.h"

#ifndef countof
#define countof(a) (sizeof(a)/ sizeof(*a))
#endif

#ifndef debug_log
#define debug_log    printf
#endif

#define VK_CHECK(vk_call) do {VkResult res = vk_call; if (res != VK_SUCCESS) {debug_log("%s:%i:%s:%s --> %s(%i)\n", __FILE__, __LINE__, __FUNCTION__, #vk_call, vk_result_to_str(res), res); fflush(stdout); exit(1); }} while(0)

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
   float values[2];
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

#define DEBUG_VEC4(v) do {debug_log("%-40s : (%f,%f,%f,%f)\n", #v, v.x, v.y, v.z, v.w); fflush(stdout); } while(0)

enum
{
   VK_SRC_ALPHA            = VK_BLEND_FACTOR_SRC_ALPHA,
   VK_ONE_MINUS_SRC_ALPHA  = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA,
   VK_COLOR_COMPONENT_ALL  = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT,
   VK_ONE_TIME_SUBMIT      = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
   VK_RENDER_PASS_CONTINUE = VK_COMMAND_BUFFER_USAGE_RENDER_PASS_CONTINUE_BIT,
};
