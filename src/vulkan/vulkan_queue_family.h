#pragma once
#include <volk.h>
#include "types.h"

namespace get
{
    class vulkan_queue_family
    {
    public:

        vulkan_queue_family(VkPhysicalDevice device, VkSurfaceKHR surface);
        ~vulkan_queue_family() = default;
        
        vulkan_queue_family(const vulkan_queue_family&) = delete;
        vulkan_queue_family(vulkan_queue_family&&) = delete;
        vulkan_queue_family& operator=(const vulkan_queue_family&) = delete;
        vulkan_queue_family& operator=(vulkan_queue_family&&) = delete;

        [[nodiscard]] u32 get_queue_family_id() const;

    private:

        u32 _queue_count;
        u32 _graphics_queue_family_id;
    };
} 
