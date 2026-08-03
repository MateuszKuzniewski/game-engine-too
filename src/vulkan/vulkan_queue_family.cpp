#include <vector>
#include <stdexcept>
#include "vulkan_queue_family.h"

get::vulkan_queue_family::vulkan_queue_family(VkPhysicalDevice device, VkSurfaceKHR surface) 
    : _queue_count(0), _graphics_queue_family_id(0)
{
    vkGetPhysicalDeviceQueueFamilyProperties2(device, &_queue_count, nullptr);

    std::vector<VkQueueFamilyProperties2> queueFamilyProperties(_queue_count, 
            { .sType = VK_STRUCTURE_TYPE_QUEUE_FAMILY_PROPERTIES_2});

    vkGetPhysicalDeviceQueueFamilyProperties2(device, &_queue_count, queueFamilyProperties.data());

    for (size_t i = 0; i < queueFamilyProperties.size(); i++)
    {
        VkBool32 hasSupport = VK_FALSE;
        vkGetPhysicalDeviceSurfaceSupportKHR(device, i, surface, &hasSupport);

        const auto& property = queueFamilyProperties[i];
        if (property.queueFamilyProperties.queueFlags & VK_QUEUE_GRAPHICS_BIT && hasSupport)
        {
            _graphics_queue_family_id = i;
            break;
        }
        else 
        {
            throw std::runtime_error("SYSTEM: No supported family queue found");
        }
    }
}

u32 get::vulkan_queue_family::get_queue_family_id() const
{
    return _graphics_queue_family_id;
}
