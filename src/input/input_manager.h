#pragma once
#include <functional>
#include "window.h"
#include "camera.h"

namespace get 
{
    class input_manager
    {
    public:

        input_manager(const window& win, camera& cam);
        ~input_manager() = default;

        input_manager(const input_manager&) = delete;
        input_manager(input_manager&&) = delete;
        input_manager& operator=(const input_manager&) = delete;
        input_manager& operator=(input_manager&&) = delete;

        void update();

    private:
    
        static void key_callback(GLFWwindow* window, int key, int scancode, int action, int mods);

    private:
        static inline i32 _current_key;

        std::unordered_map<i32, std::function<void()>> _input;
        
        GLFWwindow* _window;
        camera& _camera;
    };
}
