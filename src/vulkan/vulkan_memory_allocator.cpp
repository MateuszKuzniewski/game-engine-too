#include <stdexcept>
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

    VkResult res = vmaCreateAllocator(&vmaAllocatorInfo, &_vma_allocator);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create VMA");
    }
}

get::vulkan_memory_allocator::~vulkan_memory_allocator()
{
    if (_vma_allocator)
        vmaDestroyAllocator(_vma_allocator);
}
