#pragma once
#include <volk.h>
#include <glm/glm.hpp>
#include "vk_mem_alloc.h"
#include "types.h"

namespace get
{
    struct vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec3 color;
        glm::vec2 uv;
    };

    struct image
    {
        i32 width;
        i32 height;
        i32 channels;
        unsigned char *data;
    };

    struct gpu_buffer
    {
        VkBuffer buffer;
        u64 device_adress;
        VmaAllocation allocation;
    };

    struct gpu_image
    {
        VkImage image;
        VkImageView image_view;
        VmaAllocation allocation;
    };

    struct texture
    {
        u32 image_id;
        u32 sampler_id;
    };
}
