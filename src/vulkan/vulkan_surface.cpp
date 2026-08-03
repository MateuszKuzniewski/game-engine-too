#include <stdexcept>
#include "vulkan_surface.h"

get::vulkan_surface::vulkan_surface(const window& window, VkInstance instance) : _instance(instance)
{
    VkResult res = glfwCreateWindowSurface(instance, window.get_current_window(), nullptr, &_surface);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create Vulkan surface");
    }   
}

get::vulkan_surface::~vulkan_surface()
{
    vkDestroySurfaceKHR(_instance, _surface, nullptr);
}

VkSurfaceKHR get::vulkan_surface::get_surface() const
{
    return _surface;
}
