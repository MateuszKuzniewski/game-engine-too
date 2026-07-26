#pragma once
#include "GLFW/glfw3.h"
#include <stdexcept>

namespace get
{
    class glfw_context
    {
    public:

        glfw_context()
        {
            if (!glfwInit())
            {
                throw std::runtime_error("SYSTEM: Failed to initialize GLFW");
            }
        }

        ~glfw_context()
        {
            glfwTerminate();
        }
    };
}
