#pragma once
#include <memory>
#include "window.h"
#include "vulkan_surface.h"
#include "vulkan_context.h"
#include "glfw_context.h"

namespace get
{   
    class application
    {
    public:

        application();
        ~application();
        
        application(const application&) = delete;
        application(application&&) = delete;
        application& operator=(const application&) = delete;
        application& operator=(application&&) = delete;
    
        void run();
        void shutdown();

    private:
        std::unique_ptr<glfw_context> _glfw_context;
        std::unique_ptr<vulkan_context> _vulkan_context;
        std::unique_ptr<window> _window;
        std::unique_ptr<vulkan_surface> _surface;
    };
}
