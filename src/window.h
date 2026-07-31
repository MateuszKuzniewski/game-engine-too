#pragma once
#include <GLFW/glfw3.h>
#include <string>
#include "types.h"

namespace get
{

    struct window_settings
    {
        i32 width;
        i32 height;
        std::string title;
    };


    class window
    {
    public:

        window(const window_settings& settings);
        ~window();
    
        // TO DO: Figure out what to do with this later
        window(const window&) = delete;
        window(window&&) = delete;
        window& operator=(const window&) = delete;
        window& operator=(window&&) = delete;

        [[nodiscard]] GLFWwindow* get_current_window() const;
    
    private:
        GLFWwindow* _glfwWindow; 
        i32 _width;
        i32 _height;
    };
}
