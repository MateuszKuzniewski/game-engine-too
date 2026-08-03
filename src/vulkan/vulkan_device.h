#pragma once
#include <volk.h>
#include "types.h"

namespace get
{
    struct vulkan_feature_chain
    {
        VkPhysicalDeviceVulkan14Features features14
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_4_FEATURES, .pNext = nullptr
        };
        VkPhysicalDeviceVulkan13Features features13
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_3_FEATURES, .pNext = &features14
        };
        VkPhysicalDeviceVulkan12Features features12
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_VULKAN_1_2_FEATURES, .pNext = &features13
        };
        VkPhysicalDeviceFeatures2 features2
        {
            .sType = VK_STRUCTURE_TYPE_PHYSICAL_DEVICE_FEATURES_2, .pNext = &features12
        };

        VkPhysicalDeviceFeatures2* head() { return &features2; }
    };

    class vulkan_device
    {
    public: 

        vulkan_device(VkPhysicalDevice device, u32 queueFamilyId);
        ~vulkan_device();

    private:
        void check_supported_features(VkPhysicalDevice device) const;

    private:

        VkDevice _device;
    };
}
