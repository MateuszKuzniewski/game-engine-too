#pragma once
#include <volk.h>
#include <vk_mem_alloc.h>
#include "types.h"

namespace get
{
    class depth_buffer
    {
    public:

        depth_buffer(VkDevice device, VmaAllocator allocator, u32 width, u32 height);
        ~depth_buffer();

        depth_buffer(const depth_buffer&) = delete;
        depth_buffer(depth_buffer&&) = delete;
        depth_buffer& operator=(const depth_buffer&) = delete;
        depth_buffer& operator=(depth_buffer&&) = delete;

    private:

        VkDevice _device;
        VmaAllocator _allocator;
        VkFormat _depth_format;
        VkImage _depth_image;
        VkImageView _depth_image_view;
        VmaAllocation _depth_image_allocation;
    };
} 
