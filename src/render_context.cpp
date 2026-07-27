#include <stdexcept>
#include "types.h"
#include "render_context.h"

get::render_context::render_context(const std::string& title)
{
    if (!glfwInit())
    {
        throw std::runtime_error("SYSTEM: Failed to initialize GLFW");
    }

    VkResult res = volkInitialize();

    if (res != VK_SUCCESS)
    {
        
        throw std::runtime_error("SYSTEM: Failed to initialize Volk");
    }
    
    VkApplicationInfo app_info
    {
        .sType = VK_STRUCTURE_TYPE_APPLICATION_INFO,
        .pNext = nullptr,
        .pApplicationName = title.c_str(),
        .applicationVersion = VK_MAKE_VERSION(1, 0, 0),
        .pEngineName = "No engine",
        .engineVersion = VK_MAKE_VERSION(1,0,0),
        .apiVersion = VK_API_VERSION_1_4
    };

    u32 glfw_extension_count = 0;
    const char** glfw_extensions;

    glfw_extensions = glfwGetRequiredInstanceExtensions(&glfw_extension_count);


    VkInstanceCreateInfo create_info
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = nullptr,
        .flags = 0,
        .pApplicationInfo = &app_info,
        .enabledExtensionCount = glfw_extension_count,
        .ppEnabledExtensionNames = glfw_extensions,
    };

    res = vkCreateInstance(&create_info, nullptr, &_instance);

    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create vulkan instance");
    }

    volkLoadInstance(_instance);

}

get::render_context::~render_context()
{
    vkDestroyInstance(_instance, nullptr);
    glfwTerminate();
}
