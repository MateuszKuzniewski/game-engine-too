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

    _glfw_context = std::make_unique<render_context>();
    _window = std::make_unique<window>(settings);
}

get::application::~application()
{
    std::println("{0}", "SYSTEM: Application was destroyed");
}

void get::application::run()
{
    int x = 0;
    while(!glfwWindowShouldClose(_window->GetCurrentWindow()))
    {
        if (x >= 1500000)
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
