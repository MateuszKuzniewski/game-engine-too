#include <stdexcept>
#include "command_buffer.h"

get::command_buffer::command_buffer(VkDevice device, std::vector<frame_resource>& resources)
{
    for (auto& res : resources)
    {
        VkCommandBufferAllocateInfo allocateInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
            .commandPool = res.command_pool,
            .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
            .commandBufferCount = 1
        };

        VkResult result = vkAllocateCommandBuffers(device, &allocateInfo, &res.command_buffer);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("SYSTEM: Failed to allocate command buffer");
        }
    }
}
