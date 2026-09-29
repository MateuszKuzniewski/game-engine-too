#pragma once
#include <volk.h>
#include <vector>
#include "types.h"
#include "render_data.h"

namespace get
{
    class vk_semaphore
    {
    public:

        vk_semaphore(VkDevice device, std::vector<frame_resource>& frameResources, u32 maxFramesInFlight);
        ~vk_semaphore();

        [[nodiscard]] VkSemaphore get_semaphore();

    private:

        VkDevice _device;
        VkSemaphore _timeline_semaphore;
    };
}
