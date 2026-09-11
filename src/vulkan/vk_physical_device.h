#pragma once
#include <volk.h>
#include "types.h"

namespace get
{
    class vk_physical_device
    {
    public:

        vk_physical_device(VkInstance instance);
        ~vk_physical_device() = default;

        vk_physical_device(const vk_physical_device&) = delete;
        vk_physical_device(vk_physical_device&&) = delete;
        vk_physical_device& operator=(const vk_physical_device&) = delete;
        vk_physical_device& operator=(vk_physical_device&&) = delete;

        [[nodiscard]] VkPhysicalDevice get_device() const;

    private:
        u32 _device_count;
        VkInstance _instance;
        VkPhysicalDevice _device;
    };
}
