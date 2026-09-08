#include <stdexcept>
#include <cstring>
#include "vulkan_memory_allocator.h"

get::vulkan_memory_allocator::vulkan_memory_allocator(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device)
{
    VmaVulkanFunctions vmaFunctionsInfo{};
    VmaAllocatorCreateInfo vmaAllocatorInfo
    {
        .flags = VMA_ALLOCATOR_CREATE_BUFFER_DEVICE_ADDRESS_BIT,
        .physicalDevice = physicalDevice,
        .device = device,
        .pVulkanFunctions = &vmaFunctionsInfo,
        .instance = instance,
        .vulkanApiVersion = VK_MAKE_VERSION(1, 0, 0)
    };

    vmaImportVulkanFunctionsFromVolk(&vmaAllocatorInfo, &vmaFunctionsInfo);

    VkResult res = vmaCreateAllocator(&vmaAllocatorInfo, &_allocator);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create VMA");
    }
}

get::gpu_buffer get::vulkan_memory_allocator::create_buffer(const VkBufferCreateInfo* bufferInfo, const VmaAllocationCreateInfo* allocInfo)
{
    get::gpu_buffer result {};
    VkResult res = vmaCreateBuffer(_allocator, bufferInfo, allocInfo, &result.buffer, &result.allocation, nullptr);

    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create buffer");
        return get::gpu_buffer{};
    }

    return result;
}


get::gpu_image get::vulkan_memory_allocator::create_image(const VkImageCreateInfo* createInfo, const VmaAllocationCreateInfo* allocInfo)
{
    get::gpu_image result {};
    VkResult res = vmaCreateImage(_allocator, createInfo, allocInfo, &result.image, &result.allocation, nullptr);

    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create image");
        return get::gpu_image{};
    }

    return result;
}

void get::vulkan_memory_allocator::copy_buffer_data(const get::gpu_buffer& buffer, size_t bufferOffset, void* data, size_t byteSize)
{
    void* bufferPtr = nullptr;
    if (vmaMapMemory(_allocator, buffer.allocation, &bufferPtr) != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Unable to map buffer memory");
        return;
    }

    std::memcpy(static_cast<char*>(bufferPtr) + bufferOffset, data, byteSize);
    vmaUnmapMemory(_allocator, buffer.allocation);
}

void get::vulkan_memory_allocator::destroy(VkBuffer buffer, VmaAllocation allocation)
{
    vmaDestroyBuffer(_allocator, buffer, allocation);
}

get::vulkan_memory_allocator::~vulkan_memory_allocator()
{
    if (_allocator)
        vmaDestroyAllocator(_allocator);
}

VmaAllocator get::vulkan_memory_allocator::get_allocator() const
{
    return _allocator;
}
