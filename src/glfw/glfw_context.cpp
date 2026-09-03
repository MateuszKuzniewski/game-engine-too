#include <stdexcept>
#include "glfw_context.h"

get::glfw_context::glfw_context()
{
    // renderdoc doesn't like wayland, force to X11 if debugging
    // glfwInitHint(GLFW_PLATFORM, GLFW_PLATFORM_X11);
    int res = glfwInit();

    if (res != GL_TRUE)
    {
        throw std::runtime_error("Failed to init GLFW");
    }
}

get::glfw_context::~glfw_context()
{
    glfwTerminate(); 
}

const char** get::glfw_context::get_glfw_extensions(u32* count) const
{
    return glfwGetRequiredInstanceExtensions(count);
}
