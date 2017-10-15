
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

struct
{
   instance_t instance;
   physical_device_t gpu;
   device_t dev;
   surface_t surface;
   swapchain_t chain;
   vk_texture_t tex;   
   vk_buffer_t vbo;
   //   buffer_t  ubo;
   vk_descriptor_t desc;
   VkDescriptorSet frame_desc;
   vk_pipeline_t pipe;
   VkCommandBuffer cmd;
   VkFence queue_fence;
   VkFence chain_fence;
} vk;

static instance_t instance;
static physical_device_t gpu;
static device_t dev;
static surface_t surface;
static swapchain_t chain;
static vk_texture_t tex;
static vk_buffer_t vbo;
//static buffer_t  ubo;
static vk_descriptor_t desc;
static VkDescriptorSet frame_desc;
static vk_pipeline_t pipe;
static VkCommandBuffer cmd;
static VkFence queue_fence;
static VkFence chain_fence;
static VkFence display_fence;

void video_init()
{
   debug_log("video init\n");
   instance_init(&instance);

   physical_device_init(instance.handle, &gpu);

   device_init(gpu.handle, &dev);

   video.screen.width = 640;
   video.screen.height = 480;
#ifdef VK_USE_PLATFORM_XLIB_KHR
   video.screen.display = XOpenDisplay(NULL);
   video.screen.window  = XCreateSimpleWindow(video.screen.display,
                          DefaultRootWindow(video.screen.display), 0, 0, video.screen.width, video.screen.height, 0, 0, 0);
   XStoreName(video.screen.display, video.screen.window, "Vulkan Test");
   XSelectInput(video.screen.display, video.screen.window, ExposureMask | FocusChangeMask | KeyPressMask | KeyReleaseMask);
   XMapWindow(video.screen.display, video.screen.window);
#endif

   {
      surface_init_info_t info =
      {
         .gpu = gpu.handle,
         .queue_family_index = dev.queue_family_index,
         .width = video.screen.width,
         .height = video.screen.height,
#ifdef VK_USE_PLATFORM_XLIB_KHR
         .display = video.screen.display,
         .window = video.screen.window
#endif
      };
      surface_init(instance.handle, &info, &surface);
   }

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
      const vertex_t vertices[] =
      {
         {{ -1.0f, -1.0f, 0.0f, 1.0f}, {0.0f, 0.0f}, {1.0f, 0.0f, 0.0f, 1.0f}},
         {{ 1.0f, -1.0f, 0.0f, 1.0f}, {1.0f, 0.0f}, {0.0f, 1.0f, 0.0f, 1.0f}},
         {{ 1.0f,  1.0f, 0.0f, 1.0f}, {1.0f, 1.0f}, {0.0f, 0.0f, 1.0f, 1.0f}},
         {{ -1.0f,  1.0f, 0.0f, 1.0f}, {0.0f, 1.0f}, {1.0f, 0.0f, 0.0f, 1.0f}}
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

   descriptors_init(dev.handle, &desc);

   {
      const VkDescriptorSetAllocateInfo info =
      {
         VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
         .descriptorPool = desc.pool,
         .descriptorSetCount = 1, &desc.set_layout
      };
      vkAllocateDescriptorSets(dev.handle, &info, &frame_desc);
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

   {
      VkDisplayEventInfoEXT displayEventInfo =
      {
         VK_STRUCTURE_TYPE_DISPLAY_EVENT_INFO_EXT,
         .displayEvent = VK_DISPLAY_EVENT_TYPE_FIRST_PIXEL_OUT_EXT
      };
      VK_CHECK(vkRegisterDisplayEventEXT(dev.handle, (void*)((uintptr_t)surface.display), &displayEventInfo, NULL, &display_fence));
   }
   exit(0);

   //   uniforms_t *uniforms = ubo.mem.ptr;
   //   uniforms->center.x = 0.0f;
   //   uniforms->center.y = 0.0f;
   //   uniforms->image.width  = tex.width;
   //   uniforms->image.height = tex.height;
   //   uniforms->screen.width  = chain.viewport.width;
   //   uniforms->screen.height = chain.viewport.height;
   //   uniforms->angle = 0.0f;
}

void video_frame_update()
{
   uint32_t image_index;
   vkWaitForFences(dev.handle, 1, &chain_fence, VK_TRUE, -1);
   vkResetFences(dev.handle, 1, &chain_fence);
   vkAcquireNextImageKHR(dev.handle, chain.handle, UINT64_MAX, NULL, chain_fence, &image_index);

   vkWaitForFences(dev.handle, 1, &queue_fence, VK_TRUE, -1);
   vkResetFences(dev.handle, 1, &queue_fence);

//   vkWaitForFences(dev.handle, 1, &display_fence, VK_TRUE, -1);
//   vkResetFences(dev.handle, 1, &display_fence);

   {
      const VkCommandBufferBeginInfo info =
      {
         VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
         .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT
      };
      vkBeginCommandBuffer(cmd, &info);
   }

   device_memory_flush(dev.handle, &tex.staging.mem);
   tex.dirty = true;

   if (tex.dirty)
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
      vkCmdBindDescriptorSets(cmd, VK_PIPELINE_BIND_POINT_GRAPHICS, pipe.layout, 0, 1, &frame_desc, 0, NULL);
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
   {
      uint64_t vblank_counter = 0;
      VK_CHECK(vkGetSwapchainCounterEXT(dev.handle, chain.handle, VK_SURFACE_COUNTER_VBLANK_EXT, &vblank_counter));
      printf("vblank_counter : %lu\n", vblank_counter);
   }
}

void video_destroy()
{
   vkWaitForFences(dev.handle, 1, &queue_fence, VK_TRUE, UINT64_MAX);
   vkDestroyFence(dev.handle, queue_fence, NULL);

   vkWaitForFences(dev.handle, 1, &chain_fence, VK_TRUE, UINT64_MAX);
   vkDestroyFence(dev.handle, chain_fence, NULL);

//   vkWaitForFences(dev.handle, 1, &display_fence, VK_TRUE, UINT64_MAX);
//   vkDestroyFence(dev.handle, display_fence, NULL);

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

   XDestroyWindow(video.screen.display, video.screen.window);
   XCloseDisplay(video.screen.display);
   video.screen.display = NULL;

   video.frame.data = NULL;
   debug_log("video destroy\n");
}

void video_frame_init(int width, int height, screen_format_t format)
{
   {
      {
         texture_init_info_t info =
         {
            .queue_family_index = dev.queue_family_index,
            .width = width,
            .height = height,
            .format = format == screen_format_RGB565 ? VK_FORMAT_R5G6B5_UNORM_PACK16 :
                      format == screen_format_ARGB5551 ? VK_FORMAT_R5G5B5A1_UNORM_PACK16 :
                      VK_FORMAT_R8G8B8A8_SRGB,
            .filter = VK_FILTER_LINEAR
         };
         texture_init(dev.handle, gpu.memoryTypes, &info, &tex);
      }

      /* texture updates are written to the stating texture then uploaded later */
      memset(tex.staging.mem.u8 + tex.staging.mem_layout.offset, 0xFF,
             tex.staging.mem_layout.size - tex.staging.mem_layout.offset);

      device_memory_flush(dev.handle, &tex.staging.mem);
      tex.dirty = true;
   }

   {
      descriptors_update_info_t info =
      {
         //         .ubo_buffer = ubo.handle,
         //         .ubo_range = ubo.size,
         .sampler = tex.sampler,
         .image_view = tex.view,
      };
      descriptors_update(dev.handle, &info, frame_desc);
   }
   video.frame.width = width;
   video.frame.height = height;
   video.frame.pitch = tex.staging.mem_layout.rowPitch / 4;
   video.frame.data = tex.staging.mem.u8 + tex.staging.mem_layout.offset;

}

const video_t video_vulkan =
{
   .init = video_init,
   .frame_init = video_frame_init,
   .frame_update = video_frame_update,
   .destroy = video_destroy
};
