
#include "vulkan_common.h"

void pipeline_init(VkDevice device, const pipeline_init_info_t *init_info, vk_pipeline_t* dst)
{
}

void pipeline_free(VkDevice device, vk_pipeline_t *pipe)
{
   vkDestroyPipelineLayout(device, pipe->layout, NULL);
   vkDestroyPipeline(device, pipe->handle, NULL);
   pipe->layout = VK_NULL_HANDLE;
   pipe->handle = VK_NULL_HANDLE;
}
