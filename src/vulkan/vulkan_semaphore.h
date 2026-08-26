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

        vulkan_sempahore(VkDevice device, std::vector<frame_resource>& frameResources, u32 maxFramesInFlight);
        ~vulkan_sempahore();

        [[nodiscard]] VkSemaphore get_semaphore();

    private:

        VkDevice _device;
        VkSemaphore _timeline_semaphore;
    };
}
