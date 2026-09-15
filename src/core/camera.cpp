#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include "camera.h"
#include "frame_time.h"


get::camera::camera(u32 width, u32 height, const camera_settings settings) 
    : _settings(settings),
      _view_matrix(0),
      _projection_matrix(0),
      _view_projection_matrix(0)

{
    update((f32)width, (f32)height);
}

void get::camera::update(f32 width, f32 height)
{
    f64 ratio = width / height;

    calculate_perspective(ratio);
    calculate_view();
}

void get::camera::calculate_perspective(f64 ratio)
{
    _projection_matrix = glm::perspectiveRH(glm::radians(_settings.fov), ratio, _settings.near_clip, _settings.far_clip);
}

void get::camera::calculate_view()
{
    _view_matrix = glm::translate(glm::mat4(1.0f), _position) * glm::mat4(_rotation);
    _view_matrix = glm::inverse(_view_matrix);
}

void get::camera::move(glm::vec3 dir)
{
    f32 dt = static_cast<f32>(frame_time::delta_time());
    _position += _settings.camera_speed *  dir * dt;
}

void get::camera::rotate(f32 angle, glm::vec3 axis)
{
    auto rot = glm::rotate(_rotation, glm::radians(angle), axis);
    _rotation = glm::mat4(rot);
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
