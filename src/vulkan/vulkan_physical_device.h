#pragma once
#include <volk.h>
#include "../types.h"

namespace get
{
    class vulkan_physical_device
    {
    public:

        vulkan_physical_device(VkInstance instance);
        ~vulkan_physical_device();

        vulkan_physical_device(const vulkan_physical_device&) = delete;
        vulkan_physical_device(vulkan_physical_device&&) = delete;
        vulkan_physical_device& operator=(const vulkan_physical_device&) = delete;
        vulkan_physical_device& operator=(vulkan_physical_device&&) = delete;

        [[nodiscard]] VkPhysicalDevice get_device() const;

    private:
        u32 _device_count;
        VkInstance _instance;
        VkPhysicalDevice _device;
    };
}
