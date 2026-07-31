#include <stdexcept>
#include <vector>
#include "vulkan_physical_device.h"

get::vulkan_physical_device::vulkan_physical_device(VkInstance instance) 
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

        if (properties.deviceType == VK_PHYSICAL_DEVICE_TYPE_DISCRETE_GPU)
        {
            _device = dev;
            break;
        }
    }
}

get::vulkan_physical_device::~vulkan_physical_device()
{

}


VkPhysicalDevice get::vulkan_physical_device::get_device() const
{
    return _device;
}
