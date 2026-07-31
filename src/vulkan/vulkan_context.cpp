#include <stdexcept>
#include <string>
#include <print>
#include <iostream>
#include "types.h"
#include "vulkan_context.h"

get::vulkan_context::vulkan_context(const glfw_context& context, const std::string& title)
{
    create_vulkan_instance(context, title);
}

get::vulkan_context::~vulkan_context()
{
    if (_instance)
    {
        vkDestroyInstance(_instance, nullptr);
    }

    volkFinalize();
}

void get::vulkan_context::create_vulkan_instance(const glfw_context& context, const std::string& title)
{
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

    auto vulkan_extensions = get_vulkan_extensions(context);
    auto vulkan_validation_layers = get_vulkan_validation_layers();
    
    VkDebugUtilsMessengerCreateInfoEXT debug_info
    {
        .sType = VK_STRUCTURE_TYPE_DEBUG_UTILS_MESSENGER_CREATE_INFO_EXT,
        .messageSeverity = VK_DEBUG_UTILS_MESSAGE_SEVERITY_VERBOSE_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT
                            | VK_DEBUG_UTILS_MESSAGE_SEVERITY_ERROR_BIT_EXT,
        .messageType = VK_DEBUG_UTILS_MESSAGE_TYPE_VALIDATION_BIT_EXT
                        | VK_DEBUG_UTILS_MESSAGE_TYPE_PERFORMANCE_BIT_EXT,
        .pfnUserCallback = debug_callback
    };

    VkInstanceCreateInfo create_info
    {
        .sType = VK_STRUCTURE_TYPE_INSTANCE_CREATE_INFO,
        .pNext = &debug_info,
        .pApplicationInfo = &app_info,
        .enabledLayerCount = static_cast<u32>(vulkan_validation_layers.size()),
        .ppEnabledLayerNames = vulkan_validation_layers.data(),
        .enabledExtensionCount = static_cast<u32>(vulkan_extensions.size()),
        .ppEnabledExtensionNames = vulkan_extensions.data(),
    };

    res = vkCreateInstance(&create_info, nullptr, &_instance);

    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create vulkan instance");
    }

    volkLoadInstance(_instance);
}


std::vector<const char*> get::vulkan_context::get_vulkan_extensions(const glfw_context& glfwContext) const
{
    u32 glfw_extension_count = 0;
    auto glfw_extensions = glfwContext.get_glfw_extensions(&glfw_extension_count);
    
    std::vector<const char*> required_extensions
    {
        VK_EXT_DEBUG_UTILS_EXTENSION_NAME
    };
    
    // TO DO: Move this out of here when logging gets better
    std::println("------------------------------");
    std::println("VULKAN: Installed extensions: ");
    for (u32 i = 0; i < glfw_extension_count; ++i)
    {
        auto ext = glfw_extensions[i];
        required_extensions.push_back(ext);
        std::println(" -{0}", ext);
    }
    std::println("------------------------------");

    return required_extensions;
};

std::vector<const char*> get::vulkan_context::get_vulkan_validation_layers() const 
{
    std::vector<const char*> validation_layers
    {
        "VK_LAYER_KHRONOS_validation"
    };

    return validation_layers;
}

VKAPI_ATTR VkBool32 VKAPI_CALL get::vulkan_context::debug_callback(
    VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
    VkDebugUtilsMessageTypeFlagsEXT messageType,
    const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
    void* pUserData)
{
    if (messageSeverity >= VK_DEBUG_UTILS_MESSAGE_SEVERITY_WARNING_BIT_EXT)
    {
        std::cerr << "Validation layer: " << pCallbackData->pMessage << std::endl;
    }

    return VK_FALSE;
}
