#include <stdexcept>
#include <print>
#include "window.h"

get::window::window(const window_settings& settings) : _width(settings.width), _height(settings.height)
{
    glfwWindowHint(GLFW_CLIENT_API, GLFW_NO_API);
    _glfwWindow = glfwCreateWindow(settings.width, settings.height, settings.title.c_str(), nullptr, nullptr);
    
    if (_glfwWindow == nullptr)
    {
        throw std::runtime_error("SYSTEM: Failed to create GLFW window");
    }

    std::println("{0}", "WINDOW: Window created");
}

get::window::~window()
{
    std::println("{0}", "WINDOW: Window destroyed");
    glfwDestroyWindow(_glfwWindow);
}

GLFWwindow* get::window::GetCurrentWindow() const
{
    return _glfwWindow;
}


