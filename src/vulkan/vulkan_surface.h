#pragma once
#include <volk.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include "../window.h"

namespace get
{
    class vulkan_surface
    {
    public:
        vulkan_surface(const window& window, VkInstance instance);
        ~vulkan_surface();

    private:
        VkSurfaceKHR _surface;
        VkInstance _instance;
    };
}
