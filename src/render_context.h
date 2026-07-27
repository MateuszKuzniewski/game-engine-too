#pragma once
#include "GLFW/glfw3.h"
#include <stdexcept>

namespace get
{
    class render_context
    {
    public:

        render_context()
        {
            if (!glfwInit())
            {
                throw std::runtime_error("SYSTEM: Failed to initialize GLFW");
            }
        }

        ~render_context()
        {
            glfwTerminate();
        }
    };
}
