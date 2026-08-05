#include <memory>
#include <print>
#include "application.h"
#include "directories.h"

get::application::application()
{
    std::println("{0}", "SYSTEM: Application was created");
    std::println("{0}{1}", "SYSTEM: Project path is set to: ", get::directories::project_path());
    std::println("{0}{1}", "SYSTEM: Shader path is set to: ", get::directories::shader_path());

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

    _swapchain =        std::make_unique<vulkan_swapchain>(
                            _vulkan_device->get_device(), 
                            _physical_device->get_device(),
                            _surface->get_surface(),
                            settings.width,
                            settings.height);

    _depth_buffer =     std::make_unique<depth_buffer>(
                            _vulkan_device->get_device(), 
                            _vma->get_allocator(), 
                            settings.width, 
                            settings.height);

    _shader =           std::make_unique<shader>(
                            _vulkan_device->get_device(),
                            "shader.vert",
                            "shader.frag");

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
