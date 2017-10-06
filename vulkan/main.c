
#include <string.h>
#include "video.h"
#include "common.h"
#include "vulkan_common.h"


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

video_t video;

static instance_t instance;
static physical_device_t gpu;
static device_t dev;
static surface_t surface;
static swapchain_t chain;
static texture_t tex;
static buffer_t vbo;
//static buffer_t  ubo;
static descriptor_t desc;
static pipeline_t pipe;
static VkCommandBuffer cmd;
static VkFence queue_fence;
static VkFence chain_fence;

void video_init()
{
	debug_log("video init\n");
   instance_init(&instance);

   physical_device_init(instance.handle, &gpu);

   device_init(gpu.handle, &dev);

   {
      surface_init_info_t info =
      {
         .gpu = gpu.handle,
         .queue_family_index = dev.queue_family_index,
         .width = 640,
         .height = 480
      };
      surface_init(instance.handle, &info, &surface);
   }

   video.display = surface.display;
   video.window = surface.window;

   {
      swapchain_init_info_t info =
      {
         .surface = surface.handle,
         .width = surface.width,
         .height = surface.height,
         .present_mode = VK_PRESENT_MODE_FIFO_KHR
//         .present_mode = VK_PRESENT_MODE_IMMEDIATE_KHR
      };
      swapchain_init(dev.handle, &info, &chain);
   }

   {
      {
         texture_init_info_t info =
         {
            .queue_family_index = dev.queue_family_index,
            .width = surface.width,
            .height = surface.height,
         };
         texture_init(dev.handle, gpu.memoryTypes, &info, &tex);
      }

      /* texture updates are written to the stating texture then uploaded later */
      memset(tex.staging.mem.u8 + tex.staging.mem_layout.offset, 0xFF, tex.staging.mem_layout.size - tex.staging.mem_layout.offset);

      device_memory_flush(dev.handle, &tex.staging.mem);
      tex.dirty = true;
   }

   {
      const vertex_t vertices[] =
      {
         {{-1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
         {{ 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
         {{ 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
         {{-1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}
      };

      buffer_init_info_t info =
      {
         .usage = VK_BUFFER_USAGE_VERTEX_BUFFER_BIT,
         .size = sizeof(vertices),
         .data = vertices,
      };
      buffer_init(dev.handle, gpu.memoryTypes, &info, &vbo);
   }

//   {
//      buffer_init_info_t info =
//      {
//         .usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT,
//         .size = sizeof(uniforms_t),
//      };
//      buffer_init(dev.handle, gpu.memoryTypes, &info, &ubo);
//   }

   {
      descriptors_init_info_t info =
      {
//         .ubo_buffer = ubo.handle,
//         .ubo_range = ubo.size,
         .sampler = tex.sampler,
         .image_view = tex.view,
      };
      descriptors_init(dev.handle, &info, &desc);
   }

   {
      VkShaderModule vertex_shader;
      {
         static const uint32_t code [] =
         #include "main.vert.inc"
         ;
         const VkShaderModuleCreateInfo info =
         {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof(code),
            .pCode = code
         };
         vkCreateShaderModule(dev.handle, &info, NULL, &vertex_shader);
      }

      VkShaderModule fragment_shader;
      {
         static const uint32_t code [] =
         #include "main.frag.inc"
         ;
         const VkShaderModuleCreateInfo info =
         {
            VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
            .codeSize = sizeof(code),
            .pCode = code
         };
         vkCreateShaderModule(dev.handle, &info, NULL, &fragment_shader);
      }

      const VkVertexInputAttributeDescription attrib_desc[] =
      {
         {0, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, position)},
         {1, 0, VK_FORMAT_R32G32_SFLOAT,       offsetof(vertex_t, texcoord)},
         {2, 0, VK_FORMAT_R32G32B32A32_SFLOAT, offsetof(vertex_t, color)}
      };

      {
         pipeline_init_info_t info =
         {
            .vertex_shader = vertex_shader,
            .fragment_shader = fragment_shader,
            .vertex_size = sizeof(vertex_t),
            .attrib_count = countof(attrib_desc),
            .attrib_desc = attrib_desc,
            .set_layout = desc.set_layout,
            .scissor = &chain.scissor,
            .viewport = &chain.viewport,
            .renderpass = chain.renderpass,
         };
         pipeline_init(dev.handle, &info, &pipe);
      }

      vkDestroyShaderModule(dev.handle, vertex_shader, NULL);
      vkDestroyShaderModule(dev.handle, fragment_shader, NULL);
   }

   {
      const VkCommandBufferAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
         .commandPool = dev.cmd_pool,
         .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
         .commandBufferCount = 1
      };
      vkAllocateCommandBuffers(dev.handle, &info, &cmd);
   }

   {
      VkFenceCreateInfo info =
      {
         VK_STRUCTURE_TYPE_FENCE_CREATE_INFO,
         .flags = VK_FENCE_CREATE_SIGNALED_BIT
      };
      vkCreateFence(dev.handle, &info, NULL, &chain_fence);
      vkCreateFence(dev.handle, &info, NULL, &queue_fence);
   }


//   uniforms_t *uniforms = ubo.mem.ptr;
//   uniforms->center.x = 0.0f;
//   uniforms->center.y = 0.0f;
//   uniforms->image.width  = tex.width;
//   uniforms->image.height = tex.height;
//   uniforms->screen.width  = chain.viewport.width;
//   uniforms->screen.height = chain.viewport.height;
//   uniforms->angle = 0.0f;

}

void video_frame()
{
   uint32_t image_index;
   vkWaitForFences(dev.handle, 1, &chain_fence, VK_TRUE, -1);
   vkResetFences(dev.handle, 1, &chain_fence);
   vkAcquireNextImageKHR(dev.handle, chain.handle, UINT64_MAX, NULL, chain_fence, &image_index);

   vkWaitForFences(dev.handle, 1, &queue_fence, VK_TRUE, -1);
   vkResetFences(dev.handle, 1, &queue_fence);

   {
      const VkCommandBufferBeginInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
      };
      vkBeginCommandBuffer(cmd, &info);
   }

   if(tex.dirty)
      texture_update(cmd, &tex);

   /* renderpass */
   {
      {
         const VkClearValue clearValue = {{{0.0f, 0.1f, 1.0f, 0.0f}}};
         const VkRenderPassBeginInfo info =
         {
            VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO,
            .renderPass = chain.renderpass,
            .framebuffer = chain.framebuffers[image_index],
            .renderArea = chain.scissor,
            .clearValueCount = 1,
            .pClearValues = &clearValue
         };
         vkCmdBeginRenderPass(cmd, &info, VK_SUBPASS_CONTENTS_INLINE);
      }

      vkCmdBindPipeline(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.handle);
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.layout, 0, 1, &desc.set, 0 , NULL);
//         vkCmdPushConstants(device.cmd, device.pipelineLayout, VK_SHADER_STAGE_VERTEX_BIT, 0, sizeof(uniforms_t), mapped_uniforms);

      VkDeviceSize offset = 0;
      vkCmdBindVertexBuffers(cmd, 0, 1, &vbo.handle, &offset);

      vkCmdDraw(cmd, vbo.size / sizeof(vertex_t), 1, 0, 0);

      vkCmdEndRenderPass(cmd);
   }

   vkEndCommandBuffer(cmd);

   {
      const VkSubmitInfo info =
      {
         VK_STRUCTURE_TYPE_SUBMIT_INFO,
         .commandBufferCount = 1,
         .pCommandBuffers = &cmd
      };
      vkQueueSubmit(dev.queue, 1, &info, queue_fence);
   }

   {
      const VkPresentInfoKHR info =
      {
         VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
         .swapchainCount = 1,
         .pSwapchains = &chain.handle,
         .pImageIndices = &image_index
      };
      vkQueuePresentKHR(dev.queue, &info);
   }

}

void video_destroy()
{
   vkWaitForFences(dev.handle, 1, &queue_fence, VK_TRUE, UINT64_MAX);
   vkDestroyFence(dev.handle, queue_fence, NULL);

   vkWaitForFences(dev.handle, 1, &chain_fence, VK_TRUE, UINT64_MAX);
   vkDestroyFence(dev.handle, chain_fence, NULL);

   pipeline_free(dev.handle, &pipe);
   descriptors_free(dev.handle, &desc);
//   buffer_free(dev.handle, &ubo);
   buffer_free(dev.handle, &vbo);
   texture_free(dev.handle, &tex);
   swapchain_free(dev.handle, &chain);
   surface_free(instance.handle, &surface);
   device_free(&dev);
   physical_device_free(&gpu);
   instance_free(&instance);
	debug_log("video destroy\n");
}