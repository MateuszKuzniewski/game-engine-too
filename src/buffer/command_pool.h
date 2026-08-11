#pragma once
#include <volk.h>
#include <vector>
#include "frame_resources.h"
#include "types.h"

namespace get
{
    class command_pool
    {
    public:

        command_pool(VkDevice device, const u32 queueFamilyIndex, std::vector<frame_resource>& res);
        ~command_pool() = default;
    };
}
