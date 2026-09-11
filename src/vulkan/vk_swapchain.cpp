#include "vk_swapchain.h"
#include <stdexcept>

get::vk_swapchain::vk_swapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface) 
    :   _swapchain_format(VK_FORMAT_B8G8R8A8_SRGB), 
        _color_space(VK_COLORSPACE_SRGB_NONLINEAR_KHR),
        _device(device),
        _surface(surface),
        _physical_device(physicalDevice),
        _image_count(0)
{
}

get::vk_swapchain::~vk_swapchain()
{
    destroy();
}


void get::vk_swapchain::create(u32 width, u32 height)
{
    VkSurfaceCapabilitiesKHR surfaceCapabilites{};
    VkResult res = vkGetPhysicalDeviceSurfaceCapabilitiesKHR(_physical_device, _surface, &surfaceCapabilites);
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
        .surface = _surface,
        .minImageCount = requestedImageCount,
        .imageFormat = _swapchain_format,
        .imageColorSpace = _color_space,
        .imageExtent { .width = width, .height = height },
        .imageArrayLayers = 1,
        .imageUsage = VK_IMAGE_USAGE_COLOR_ATTACHMENT_BIT,
        .preTransform = surfaceCapabilites.currentTransform,
        .compositeAlpha = VK_COMPOSITE_ALPHA_OPAQUE_BIT_KHR,
        .presentMode = VK_PRESENT_MODE_FIFO_KHR
    };
    
    res = vkCreateSwapchainKHR(_device, &swapchainCreateInfo, nullptr, &_swapchain); 

    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create swapchain");
    }

    vkGetSwapchainImagesKHR(_device, _swapchain, &_image_count, nullptr);
    _swapchain_images.resize(_image_count);

    vkGetSwapchainImagesKHR(_device, _swapchain, &_image_count, _swapchain_images.data());
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

        res = vkCreateImageView(_device, &imageViewInfo, nullptr, &_swapchain_image_views[i]);
        
        if (res != VK_SUCCESS)
        {
            throw std::runtime_error("SYSTEM: Failed to create swapchain image view");
        }
    }

    _render_complete_semaphores.resize(_swapchain_images.size());
    for (auto& semaphore : _render_complete_semaphores)
    {
        VkSemaphoreCreateInfo semaphoreInfo { .sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO };
        
        res = vkCreateSemaphore(_device, &semaphoreInfo, nullptr, &semaphore);
        if (res != VK_SUCCESS)
        {
            throw std::runtime_error("SYSTEM: Failed to create render-complete semaphore");
        }
    }
}

void get::vk_swapchain::destroy()
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

VkSwapchainKHR get::vk_swapchain::get_swapchain()
{
    return _swapchain;
}

std::vector<VkImage>& get::vk_swapchain::get_swapchain_images()
{
    return _swapchain_images;
}

std::vector<VkImageView>& get::vk_swapchain::get_swapchain_image_views()
{
    return _swapchain_image_views;
}

std::vector<VkSemaphore>& get::vk_swapchain::get_render_complete_semaphores()
{
    return _render_complete_semaphores;
}
