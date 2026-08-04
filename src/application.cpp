#include <memory>
#include <print>
#include "application.h"

get::application::application()
{
    std::println("{0}", "SYSTEM: Application was created");
    get::window_settings settings
    {
        .width = 1280,
        .height = 720,
        .title = "Game Engine Too"
    };
    
    _glfw_context =     std::make_unique<glfw_context>();
    _vulkan_context =   std::make_unique<vulkan_context>(*_glfw_context, settings.title);
    _window =           std::make_unique<window>(settings);
    _surface =          std::make_unique<vulkan_surface>(*_window, _vulkan_context->get_instance());
    _physical_device =  std::make_unique<vulkan_physical_device>(_vulkan_context->get_instance());
    _queue_family =     std::make_unique<vulkan_queue_family>(_physical_device->get_device(), _surface->get_surface());
    _vulkan_device =    std::make_unique<vulkan_device>(_physical_device->get_device(), _queue_family->get_queue_family_id());
    _vma =              std::make_unique<vulkan_memory_allocator>(
                             _vulkan_context->get_instance(), 
                             _physical_device->get_device(), 
                             _vulkan_device->get_device());
}

get::application::~application()
{
    std::println("{0}", "SYSTEM: Application was destroyed");
}

void get::application::run()
{
    int x = 0;
    while(!glfwWindowShouldClose(_window->get_current_window()))
    {
        if (x >= 500)
            break;
        x++;
        glfwPollEvents();
    }

    std::println("{0}", "SYSTEM: Application is running");
}

void get::application::shutdown()
{
    std::println("{0}", "SYSTEM: Application was closed");
} 
