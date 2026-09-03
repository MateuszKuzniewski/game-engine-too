#pragma once
#include <GL/gl.h>
#include <GLFW/glfw3.h>
#include "types.h"

namespace get
{
    class frame_time
    {
    public:

        frame_time() = delete;
        ~frame_time() = default;

        [[nodiscard]] inline static f64 delta_time()
        {
            f64 currentFrame = glfwGetTime();
            f64 deltaTime = currentFrame - _last_frame;
            _last_frame = currentFrame;

            return deltaTime;
        };

    private:

        inline static f64 _last_frame;
    };
}
