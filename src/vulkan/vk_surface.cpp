#include <stdexcept>
#include "vk_surface.h"

get::vk_surface::vk_surface(const window& window, VkInstance instance) : _instance(instance)
{
    VkResult res = glfwCreateWindowSurface(instance, window.get_current_window(), nullptr, &_surface);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create Vulkan surface");
    }   
}

get::vk_surface::~vk_surface()
{
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
}

VkSurfaceKHR get::vk_surface::get_surface() const
{
    return _surface;
}
