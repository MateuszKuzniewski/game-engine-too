#pragma once
#include <volk.h>
#include <vk_mem_alloc.h>
#include "render_data.h"

namespace get
{
    class vk_memory_allocator
    {
    public:

        vk_memory_allocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
        ~vk_memory_allocator();

        [[nodiscard]] VmaAllocator get_allocator() const;
        
        get::gpu_buffer create_buffer(const VkBufferCreateInfo* bufferInfo, const VmaAllocationCreateInfo* allocInfo);
        get::gpu_image create_image(const VkImageCreateInfo* createInfo, const VmaAllocationCreateInfo* allocInfo);
        
        void copy_buffer_data(const get::gpu_buffer& buffer, size_t bufferOffset, void* data, size_t byteSize);
        void destroy(VkBuffer buffer, VmaAllocation allocation);

    private:

        VmaAllocator _allocator;
    };
}
