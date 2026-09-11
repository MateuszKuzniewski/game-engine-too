#include <stdexcept>
#include <iostream>
#include <vector>
#include "vk_physical_device.h"

get::vk_physical_device::vk_physical_device(VkInstance instance) 
    : _device_count(0), _instance(instance), _device(nullptr)
{
    vkEnumeratePhysicalDevices(_instance, &_device_count, nullptr);
    std::vector<VkPhysicalDevice> physicalDevices(_device_count);
    vkEnumeratePhysicalDevices(_instance, &_device_count, physicalDevices.data());

    if (_device_count == 0)
        throw std::runtime_error("SYSTEM: No physical devices found");

    _device = physicalDevices[0];
    for (auto& dev : physicalDevices)
    {
        VkPhysicalDeviceProperties properties{};
        vkGetPhysicalDeviceProperties(dev, &properties);

        std::cout << "RENDERER: " << properties.deviceName << '\n';
        std::cout << "VULKAN VERSION: " 
            << VK_API_VERSION_MAJOR(properties.apiVersion) << "." 
            << VK_API_VERSION_MINOR(properties.apiVersion) << "." 
            << VK_API_VERSION_PATCH(properties.apiVersion) << '\n';
        

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            _device = dev;
            break;
        }
    }
}

VkPhysicalDevice get::vk_physical_device::get_device() const
{
    return _device;
}
