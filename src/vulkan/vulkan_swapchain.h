#pragma once
#include <volk.h>
#include <vector>
#include "types.h"

namespace get
{
    class vulkan_swapchain
    {
    public:

        vulkan_swapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surface, u32 width, u32 height);
        ~vulkan_swapchain();

        [[nodiscard]] VkSwapchainKHR get_swapchain() const;

    private:

        VkSwapchainKHR _swapchain;
        VkFormat _swapchain_format;
        VkColorSpaceKHR _color_space;
        VkDevice _device;
        
        
        u32 _width;
        u32 _height;
        u32 _image_count;

        std::vector<VkImage> _swapchain_images;
        std::vector<VkImageView> _swapchain_image_views;
        std::vector<VkSemaphore> _render_complete_semaphores;
    };
}
