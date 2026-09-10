#pragma once
#include <volk.h>
#include <vector>
#include <render_data.h>

namespace get
{
    class command_buffer
    {
    public:

        command_buffer(VkDevice device, std::vector<frame_resource>& resources);
        ~command_buffer() = default;
    };
}
