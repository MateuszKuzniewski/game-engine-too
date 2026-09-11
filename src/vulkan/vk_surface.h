#pragma once
#include <volk.h>
#include <GLFW/glfw3.h>
#include "window.h"

namespace get
{
    class vk_surface
    {
    public:
        vk_surface(const window& window, VkInstance instance);
        ~vk_surface();

        vk_surface(const vk_surface&) = delete;
        vk_surface(vk_surface&&) = delete;
        vk_surface& operator=(const vk_surface&) = delete;
        vk_surface& operator=(vk_surface&&) = delete;

        [[nodiscard]] VkSurfaceKHR get_surface() const;

    private:
        VkSurfaceKHR _surface;
        VkInstance _instance;
    };
}
