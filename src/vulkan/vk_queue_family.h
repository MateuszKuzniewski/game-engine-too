#pragma once
#include <volk.h>
#include "types.h"

namespace get
{
    class vk_queue_family
    {
    public:

        vk_queue_family(VkPhysicalDevice device, VkSurfaceKHR surface);
        ~vk_queue_family() = default;
        
        vk_queue_family(const vk_queue_family&) = delete;
        vk_queue_family(vk_queue_family&&) = delete;
        vk_queue_family& operator=(const vk_queue_family&) = delete;
        vk_queue_family& operator=(vk_queue_family&&) = delete;

        [[nodiscard]] u32 get_queue_family_id() const;

    private:

        u32 _queue_count;
        u32 _graphics_queue_family_id;
    };
} 
