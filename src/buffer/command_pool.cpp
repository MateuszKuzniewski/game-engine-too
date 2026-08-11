#include <stdexcept>
#include "command_pool.h"


get::command_pool::command_pool(VkDevice device, const u32 queueFamilyIndex, std::vector<frame_resource>& resources)
{

    for (auto& res : resources)
    {
        VkCommandPoolCreateInfo poolInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .queueFamilyIndex = queueFamilyIndex
        };

        VkResult result = vkCreateCommandPool(device, &poolInfo, nullptr, &res.command_pool);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("SYSTEM: Failed to create a command pool");
        }
    }
}
