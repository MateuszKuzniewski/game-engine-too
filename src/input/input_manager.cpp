#include "input_manager.h"
#include "world_data.h"

get::input_manager::input_manager(const window& win, camera& cam)
    :    _camera(cam)
{
    glfwSetKeyCallback(win.get_current_window(), input_manager::key_callback);
    
    // camera -> move
    _input[GLFW_KEY_W] =        [this]() { _camera.move(_camera.get_view_dir());    };
    _input[GLFW_KEY_S] =        [this]() { _camera.move(-_camera.get_view_dir());   };
    _input[GLFW_KEY_A] =        [this]() { _camera.move(-_camera.get_right_dir());  };
    _input[GLFW_KEY_D] =        [this]() { _camera.move(_camera.get_right_dir());   };
    _input[GLFW_KEY_SPACE] =    [this]() { _camera.move(world::up); };
    _input[GLFW_KEY_C] =        [this]() { _camera.move(world::down); };
    
    // camera -> rotate
    _input[GLFW_KEY_LEFT] =     [this]() { _camera.rotate(5.0f, world::up);       };
    _input[GLFW_KEY_RIGHT] =    [this]() { _camera.rotate(-5.0f, world::up);      }; 
    _input[GLFW_KEY_UP] =       [this]() { _camera.rotate(5.0f, world::right);    };
    _input[GLFW_KEY_DOWN] =     [this]() { _camera.rotate(-5.0f, world::right);   };
    _input[GLFW_KEY_E] =        [this]() { _camera.rotate(5.0f, world::forward);  };
    _input[GLFW_KEY_Q] =        [this]() { _camera.rotate(-5.0f, world::forward); };
}

void get::input_manager::update()
{
    for (auto key : _held_keys)
    {
        auto it = _input.find(key); 
        if (it != _input.end())
        {
            it->second();
        }
    }
}

void get::input_manager::key_callback(GLFWwindow* window, int key, int scancode, int action, int mods)
{
    key_press_event(key, action);
}

void get::input_manager::key_press_event(i32 key, i32 action)
{
    if (action == GLFW_PRESS)
    {
        _held_keys.insert(key);
    }
    else if (action == GLFW_RELEASE)
    {
        _held_keys.erase(key);
    }
}
