#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/matrix.hpp>
#include <glm/gtc/quaternion.hpp>
#include "types.h"

namespace get
{
    enum class camera_mode : i8
    {
        FREE = 0,
        ORBIT
    };

    struct camera_settings
    {
        f64 fov;
        f64 near_clip;
        f64 far_clip;
        f32 camera_speed;
    };

    class camera
    {
    public:
        
        camera(u32 width, u32 height, const camera_settings settings);
        ~camera() = default;

        void update(f32 width, f32 height);
    
        void move(glm::vec3 dir);

        void rotate(f32 angle, glm::vec3 axis);

        glm::mat4 get_view_projection_matrix();

        [[nodiscard]] glm::vec3 get_positon() const; 
        [[nodiscard]] glm::vec3 get_view_dir() const;
        [[nodiscard]] glm::vec3 get_right_dir() const;

    private:

        void calculate_perspective(f64 ratio);
        void calculate_view();

    private:
        
        camera_settings _settings;

        glm::quat _rotation = glm::quat(1,0,0,0);

        glm::vec3 _position = glm::vec3(0);

        glm::mat4 _view_matrix;
        glm::mat4 _projection_matrix;
        glm::mat4 _view_projection_matrix;
    };
}
