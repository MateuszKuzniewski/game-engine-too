#include <stdexcept>
#include "depth_buffer.h"

get::depth_buffer::depth_buffer(VkDevice device, VmaAllocator allocator, u32 width, u32 height) 
    :   _device(device),
        _allocator(allocator),
        _depth_format(VK_FORMAT_D32_SFLOAT)
{
    VkImageCreateInfo depthCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = _depth_format,
        .extent { .width = width, .height = height, .depth = 1},
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_DEPTH_STENCIL_ATTACHMENT_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo allocationInfo
    {
        .flags = VMA_ALLOCATION_CREATE_DEDICATED_MEMORY_BIT,
        .usage = VMA_MEMORY_USAGE_AUTO
    };

    VkResult res = vmaCreateImage(allocator, &depthCreateInfo, &allocationInfo, &_depth_image, &_depth_image_allocation, nullptr);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to allocate resources");
    }

    VkImageViewCreateInfo depthImageViewInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = _depth_image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = _depth_format,
        .subresourceRange = 
        {
            .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };

    res = vkCreateImageView(device, &depthImageViewInfo, nullptr, &_depth_image_view);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create image view");
    }
}

get::depth_buffer::~depth_buffer()
{
    vkDestroyImageView(_device, _depth_image_view, nullptr);
    vmaDestroyImage(_allocator, _depth_image, _depth_image_allocation);
}   
