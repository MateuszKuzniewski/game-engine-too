#pragma once
#include <volk.h>
#include <GLFW/glfw3.h>
#include "../window.h"

namespace get
{
    class vulkan_surface
    {
    public:
        vulkan_surface(const window& window, VkInstance instance);
        ~vulkan_surface();

        vulkan_surface(const vulkan_surface&) = delete;
        vulkan_surface(vulkan_surface&&) = delete;
        vulkan_surface& operator=(const vulkan_surface&) = delete;
        vulkan_surface& operator=(vulkan_surface&&) = delete;


    private:
        VkSurfaceKHR _surface;
        VkInstance _instance;
    };
}
