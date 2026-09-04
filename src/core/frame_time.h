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

        inline static void update()
        {
            f64 currentFrame = glfwGetTime();
            _delta_time = currentFrame - _last_frame;
            _last_frame = currentFrame;
        };

        [[nodiscard]] inline static f64 delta_time() { return _delta_time; };

    private:
        inline static f64 _delta_time;
        inline static f64 _last_frame;
    };
}
