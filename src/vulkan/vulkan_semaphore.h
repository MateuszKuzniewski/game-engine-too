#pragma once
#include <volk.h>
#include <vector>
#include "types.h"
#include "frame_resources.h"

namespace get
{
    class vulkan_sempahore
    {
    public:

        vulkan_sempahore(VkDevice device, std::vector<frame_resource>& frameResources);
        ~vulkan_sempahore();

    private:
        VkDevice _device;
        u32 _max_frames_in_flight;
        VkSemaphore _timeline_semaphore;
    };
}
