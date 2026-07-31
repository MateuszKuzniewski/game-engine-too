#pragma once
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "../types.h"

namespace get
{
    class glfw_context
    {
    public:

        glfw_context();
        ~glfw_context();

        glfw_context(const glfw_context&) = delete;
        glfw_context(glfw_context&&) = delete;
        glfw_context& operator=(const glfw_context&) = delete;
        glfw_context& operator=(glfw_context&&) = delete;

        const char** get_glfw_extensions(u32* count) const;

    };
}
