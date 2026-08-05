#pragma once
#include <volk.h>
#include <vk_mem_alloc.h>

namespace get
{
    class vulkan_memory_allocator
    {
    public:

        vulkan_memory_allocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device);
        ~vulkan_memory_allocator();

        [[nodiscard]] VmaAllocator get_allocator() const;

        

    private:

        VmaAllocator _vma_allocator;
    };
}
