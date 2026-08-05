#include <stdexcept>
#include <vector>
#include <array>
#include "vulkan_device.h"


get::vulkan_device::vulkan_device(VkPhysicalDevice device, u32 queueFamilyId)
{
    check_supported_features(device);
    
    vulkan_feature_chain chain;
    chain.features13.dynamicRendering = VK_TRUE;
    chain.features13.synchronization2 = VK_TRUE;
    chain.features12.timelineSemaphore = VK_TRUE;

    std::vector<f32> queuePriorities { 1.0f };
    VkDeviceQueueCreateInfo queueInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_QUEUE_CREATE_INFO,
        .queueFamilyIndex = queueFamilyId,
        .queueCount = 1,
        .pQueuePriorities = queuePriorities.data()
    };
    
    std::vector<const char*> deviceExtensions { VK_KHR_SWAPCHAIN_EXTENSION_NAME };
    VkDeviceCreateInfo deviceInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEVICE_CREATE_INFO,
        .pNext = chain.head(),
        .queueCreateInfoCount = 1,
        .pQueueCreateInfos = &queueInfo,
        .enabledExtensionCount = static_cast<u32>(deviceExtensions.size()),
        .ppEnabledExtensionNames = deviceExtensions.data(),
        .pEnabledFeatures = nullptr
    };
    
    VkResult res = vkCreateDevice(device, &deviceInfo, nullptr, &_device);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create logical device");
    }
    
    vkGetDeviceQueue(_device, queueFamilyId, 0, &_queue);

    if (!_queue)
    {
        throw std::runtime_error("SYSTEM: Failed to create vulkan queue");
    }
}

get::vulkan_device::~vulkan_device()
{
    vkDestroyDevice(_device, nullptr);
}

void get::vulkan_device::check_supported_features(VkPhysicalDevice device) const
{
    vulkan_feature_chain chain;
    vkGetPhysicalDeviceFeatures2(device, chain.head());

    const std::array<std::pair<VkBool32, const char*>, 3> required
    {{
        { chain.features13.dynamicRendering,  "dynamicRendering"  },
        { chain.features13.synchronization2,  "synchronization2"  },
        { chain.features12.timelineSemaphore, "timelineSemaphore" },
    }};

    std::string missing;
    for (const auto& [supported, name] : required)
    {
        if (!supported)
        {
            if (!missing.empty()) missing += ", ";
            missing += name;
        }
    }

    if (!missing.empty())
    {
        throw std::runtime_error("SYSTEM: Missing required Vulkan features: " + missing);
    }
}

VkDevice get::vulkan_device::get_device() const
{
    return _device;
}

VkQueue get::vulkan_device::get_queue() const
{
    return _queue;
}
