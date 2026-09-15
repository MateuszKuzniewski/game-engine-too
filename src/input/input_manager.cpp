#include "input_manager.h"
#include "world_data.h"

get::input_manager::input_manager(const window& win, camera& cam)
    :    _camera(cam)
{
    glfwSetKeyCallback(win.get_current_window(), input_manager::key_callback);
    
    // camera -> move
    _input[GLFW_KEY_W] =        [this]() { _camera.move(world::forward);    };
    _input[GLFW_KEY_S] =        [this]() { _camera.move(world::backward);   };
    _input[GLFW_KEY_A] =        [this]() { _camera.move(world::left);       };
    _input[GLFW_KEY_D] =        [this]() { _camera.move(world::right);      };
    _input[GLFW_KEY_SPACE] =    [this]() { _camera.move(world::up);         };
    _input[GLFW_KEY_C] =        [this]() { _camera.move(world::down);       };

    // camera -> rotate
    _input[GLFW_KEY_LEFT] =     [this]() { _camera.rotate(5.0f, world::up); };
    _input[GLFW_KEY_RIGHT] =    [this]() { _camera.rotate(-5.0f, world::up); };
}

void get::input_manager::update()
{
    auto it = _input.find(_current_key); 
    if (it != _input.end())
    {
        it->second();
    }
}

void get::input_manager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    _current_key = key;
}

