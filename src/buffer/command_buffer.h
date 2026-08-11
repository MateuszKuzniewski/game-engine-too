#pragma once
#include <volk.h>
#include <vector>
#include <frame_resources.h>

namespace get
{
    class command_buffer
    {
    public:

        command_buffer(VkDevice device, std::vector<frame_resource>& resources);
        ~command_buffer() = default;
    };
}
