#pragma once
#include <volk.h>
#include <vector>
#include "types.h"

namespace get
{
    class vulkan_swapchain
    {
    public:

        vulkan_swapchain(VkDevice device, VkPhysicalDevice physicalDevice, VkSurfaceKHR surfac);
        ~vulkan_swapchain();

        [[nodiscard]] VkSwapchainKHR get_swapchain();

        [[nodiscard]] std::vector<VkImage>& get_swapchain_images();
        [[nodiscard]] std::vector<VkImageView>& get_swapchain_image_views();
        [[nodiscard]] std::vector<VkSemaphore>& get_render_complete_semaphores();

        void create(u32 width, u32 height);
        void destroy();

    private:

        VkSwapchainKHR _swapchain;
        VkFormat _swapchain_format;
        VkColorSpaceKHR _color_space;
        VkDevice _device;
        VkSurfaceKHR _surface;
        VkPhysicalDevice _physical_device;
        
        u32 _image_count;

        std::vector<VkImage> _swapchain_images;
        std::vector<VkImageView> _swapchain_image_views;
        std::vector<VkSemaphore> _render_complete_semaphores;
    };
}
