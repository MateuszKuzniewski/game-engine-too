#pragma once
#include <volk.h>
#include <vector>
#include <string>
#include <vk_mem_alloc.h>
#include "../glfw/glfw_context.h"


namespace get
{
    class vulkan_context
    {
    public:
        vulkan_context(const glfw_context& glfwContext, const std::string& title);
        ~vulkan_context();

        VkInstance GetInstance() { return _instance; }

    private:  
        void create_vulkan_instance(const glfw_context& glfwContext, const std::string& title);
        
        std::vector<const char*> get_vulkan_extensions(const glfw_context& glfwContext);
        std::vector<const char*> get_vulkan_validation_layers();

        static VKAPI_ATTR VkBool32 VKAPI_CALL debug_callback(
            VkDebugUtilsMessageSeverityFlagBitsEXT messageSeverity,
            VkDebugUtilsMessageTypeFlagsEXT messageType,
            const VkDebugUtilsMessengerCallbackDataEXT* pCallbackData,
            void* pUserData);

    private:
        VkInstance _instance;
    };
}
