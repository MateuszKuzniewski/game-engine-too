#include <stdexcept>
#include "vulkan_semaphore.h"

get::vulkan_sempahore::vulkan_sempahore(VkDevice device, std::vector<frame_resource>& frameResources, u32 maxFramesInFlight) 
    : _device(device) 
{
    VkSemaphoreTypeCreateInfo semahoreTypeInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_TYPE_CREATE_INFO,
        .semaphoreType = VK_SEMAPHORE_TYPE_TIMELINE,
        .initialValue = maxFramesInFlight
    };

    VkSemaphoreCreateInfo semaphoreInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO,
        .pNext = &semahoreTypeInfo
    };

    VkResult res = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &_timeline_semaphore);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create a semaphore");
    }

    for (auto& res : frameResources)
    {
        VkSemaphoreCreateInfo semaphoreInfo 
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO
        };

        if (vkCreateSemaphore(device, &semaphoreInfo, nullptr, &res.image_acquired_semaphore) != VK_SUCCESS)
        {
            throw std::runtime_error("SYSTEM: Failed to create per-image semaphore");
        }
    }
}

get::vulkan_sempahore::~vulkan_sempahore()
{
    vkDestroySemaphore(_device, _timeline_semaphore, nullptr);
}

VkSemaphore get::vulkan_sempahore::get_semaphore()
{
    return _timeline_semaphore;
}
