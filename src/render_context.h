#pragma once
#include <volk.h>
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include <vector>
#include <string>

namespace get
{
    class render_context
    {
    public:

        render_context(const std::string& title);
        ~render_context();

    private:  

        void glfw_init(); 
        void create_vulkan_instance(const std::string& title);
        
        std::vector<const char*> get_vulkan_extensions();
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
