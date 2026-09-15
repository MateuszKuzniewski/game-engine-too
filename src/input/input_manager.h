#pragma once
#include <functional>
#include <unordered_set>
#include <unordered_map>
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
        static void key_press_event(i32 key, i32 action);

    private:
        static inline std::unordered_set<i32> _held_keys;

        std::unordered_map<i32, std::function<void()>> _input;
        GLFWwindow* _win;
        camera& _camera;
    };
}
