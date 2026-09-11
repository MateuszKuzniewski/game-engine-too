#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "camera.h"
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>

get::camera::camera(u32 width, u32 height, const camera_settings settings) 
    : _settings(settings),
      _view_matrix(0),
      _projection_matrix(0),
      _view_projection_matrix(0)

{
    update(width, height);
}

void get::camera::update(u32 width, u32 height)
{
    f32 ratio = static_cast<f32>(width) / static_cast<f32>(height);
    calculate_perspective(ratio);
    calculate_view();
}

void get::camera::calculate_perspective(f64 ratio)
{
    // TO DO: Check perspectiveRH
    _projection_matrix = glm::perspectiveRH(glm::radians(_settings.fov), ratio, _settings.near_clip, _settings.far_clip);
}

void get::camera::calculate_view()
{
    // pitch / yaw / roll
    // _roll += glm::radians(10.f) * frame_time::delta_time();
    glm::quat rot = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 0.0f); 

    _view_matrix = glm::translate(glm::mat4(1.0f), pos) * glm::mat4(rot);
    _view_matrix = glm::inverse(_view_matrix);
}

glm::mat4 get::camera::get_view_projection_matrix() 
{
    _view_projection_matrix = _projection_matrix * _view_matrix;
    return _view_projection_matrix;
}


glm::vec3 get::camera::get_positon() const
{
    return _position;
}
