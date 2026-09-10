#pragma once
#include <glm/glm.hpp>
#include <glm/matrix.hpp>
#include "types.h"

namespace get
{
    struct camera_settings
    {
        f64 fov;
        f64 near_clip;
        f64 far_clip;
    };

    class camera
    {
    public:

        camera(u32 width, u32 height, const camera_settings settings);
        ~camera() = default;

        glm::mat4 get_view_projection_matrix();
        void update(u32 width, u32 height);
        [[nodiscard]] glm::vec3 get_positon() const; 

    private:

        void calculate_perspective(f64 ratio);
        void calculate_view();

    private:
        
        glm::vec3 _position = glm::vec3(0);
        camera_settings _settings;
        f64 _roll;       
        glm::mat4 _view_matrix;
        glm::mat4 _projection_matrix;
        glm::mat4 _view_projection_matrix;
    };
}
