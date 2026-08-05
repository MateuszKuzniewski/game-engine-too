#pragma once
#include <memory>
#include "window.h"
#include "glfw_context.h"
#include "vulkan_surface.h"
#include "vulkan_context.h"
#include "vulkan_queue_family.h"
#include "vulkan_physical_device.h"
#include "vulkan_device.h"
#include "vulkan_memory_allocator.h"
#include "vulkan_swapchain.h"
#include "depth_buffer.h"
#include "shader.h"

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
        std::unique_ptr<vulkan_physical_device> _physical_device;
        std::unique_ptr<vulkan_queue_family> _queue_family;
        std::unique_ptr<vulkan_device> _vulkan_device;
        std::unique_ptr<vulkan_memory_allocator> _vma;
        std::unique_ptr<vulkan_swapchain> _swapchain;
        std::unique_ptr<depth_buffer> _depth_buffer;
        std::unique_ptr<shader> _shader;

    };
}
