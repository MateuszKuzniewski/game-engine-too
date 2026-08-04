#include "vulkan_swapchain.h"
#include <stdexcept>

get::vulkan_swapchain::vulkan_swapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, u32 width, u32 height) 
    :   _swapchain_format(VK_FORMAT_B8G8R8A8_SRGB), 
        _color_space(VK_COLORSPACE_SRGB_NONLINEAR_KHR),
        _device(device),
        _width(width), 
        _height(height),
        _image_count(0)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilites{};
    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(physicalDevice, surface, &surfaceCapabilites);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to retrieve surface capabilites");
    }

    u32 requestedImageCount = std::max(2u, surfaceCapabilites.minImageCount);
    if (surfaceCapabilites.maxImageCount > 0)
    {
        requestedImageCount = std::min(requestedImageCount, surfaceCapabilites.maxImageCount);
    }

    VkSwapchainCreateInfoKHR swapchainCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SWAPCHAIN_CREATE_INFO_KHR,
        .surface = surface,
        .minImageCount = requestedImageCount,
        .imageFormat = _swapchain_format,
        .imageColorSpace = _color_space,
        .imageExtent { .width = _width, .height = _height },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = surfaceCapabilites.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };
    
    res = vkCreateSwapchainKHR(device, &swapchainCreateInfo, nullptr, &_swapchain); 

    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create swapchain");
    }

    vkGetSwapchainImagesKHR(device, _swapchain, &_image_count, nullptr);
    _swapchain_images.resize(_image_count);

    vkGetSwapchainImagesKHR(device, _swapchain, &_image_count, _swapchain_images.data());
    _swapchain_image_views.resize(_image_count);

    for (size_t i = 0; i < _swapchain_image_views.size(); i++)
    {
        VkImageViewCreateInfo imageViewInfo
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
            .image = _swapchain_images[i],
            .viewType = VK_IMAGE_VIEW_TYPE_2D,
            .format = _swapchain_format,
            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1
            }
        };

        res = vkCreateImageView(device, &imageViewInfo, nullptr, &_swapchain_image_views[i]);
        
        if (res != VK_SUCCESS)
        {
            throw std::runtime_error("SYSTEM: Failed to create swapchain image view");
        }
    }

    _render_complete_semaphores.resize(_swapchain_images.size());
    for (auto& semaphore : _render_complete_semaphores)
    {
        VkSemaphoreCreateInfo semaphoreInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        
        res = vkCreateSemaphore(device, &semaphoreInfo, nullptr, &semaphore);
        if (res != VK_SUCCESS)
        {
            throw std::runtime_error("SYSTEM: Failed to create render-complete semaphore");
        }
    }
}

get::vulkan_swapchain::~vulkan_swapchain()
{
    for (auto& imageView : _swapchain_image_views)
    {
        vkDestroyImageView(_device, imageView, nullptr);
    }
    _swapchain_image_views.clear();

    for (auto& semaphore : _render_complete_semaphores)
    {
        vkDestroySemaphore(_device, semaphore, nullptr);
    }
    _render_complete_semaphores.clear();

    if (_swapchain)
        vkDestroySwapchainKHR(_device, _swapchain, nullptr);
}

VkSwapchainKHR get::vulkan_swapchain::get_swapchain() const
{
    return _swapchain;
}
