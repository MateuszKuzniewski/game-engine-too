#include <stdexcept>
#include "command_pool.h"


get::command_pool::command_pool(VkDevice device, const u32 queueFamilyIndex, std::vector<frame_resource>& resources)
    : _device(device), _id(queueFamilyIndex)
{
    VkCommandPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
            .flags = VK_COMMAND_POOL_CREATE_TRANSIENT_BIT,
            .queueFamilyIndex = _id,
    };

    VkResult result = vkCreateCommandPool(_device, &poolInfo, nullptr, &_transient_command_pool); 
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create a command pool");
    }

    for (auto& res : resources)
    {
        VkCommandPoolCreateInfo poolInfo
        {
            .sType = VK_STRUCTURE_TYPE_COMMAND_POOL_CREATE_INFO,
                .queueFamilyIndex = _id
        };

        VkResult result = vkCreateCommandPool(_device, &poolInfo, nullptr, &res.command_pool);
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("SYSTEM: Failed to create a command pool");
        }
    }
}

get::command_pool::~command_pool()
{
    vkDestroyCommandPool(_device, _transient_command_pool, nullptr);
}

VkCommandPool get::command_pool::get_transient_command_pool() const
{
    return _transient_command_pool;
}
