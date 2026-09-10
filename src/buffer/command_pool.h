#pragma once
#include <volk.h>
#include <vector>
#include "render_data.h"
#include "types.h"

namespace get
{
    class command_pool
    {
    public:

        command_pool(VkDevice device, const u32 queueFamilyIndex, std::vector<frame_resource>& resources);
        ~command_pool();

        void create_command_pool_resource(std::vector<frame_resource>& resources);
        void create_command_pool_transient();

        VkCommandPool get_transient_command_pool() const;

    private:

        VkDevice _device;
        u32 _id;

        VkCommandPool _transient_command_pool;
    };
}
