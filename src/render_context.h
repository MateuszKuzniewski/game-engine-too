#pragma once
#include <volk.h>
#include <GLFW/glfw3.h>
#include <vulkan/vulkan_core.h>
#include <string>

namespace get
{
    class render_context
    {
    public:

        render_context(const std::string& title);
        ~render_context();

    private:
        VkInstance _instance;
    };
}
