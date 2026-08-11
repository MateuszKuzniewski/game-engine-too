#pragma once
#include <volk.h>


namespace get
{
    struct frame_resource 
    {
        VkCommandPool command_pool = nullptr;
        VkCommandBuffer command_buffer = nullptr;
        VkSemaphore image_acquired_semaphore = nullptr;
    };
}
