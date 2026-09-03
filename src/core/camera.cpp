#include "camera.h"
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include "frame_time.h"

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
    f32 ratio = (f32)width / (f32)height;
    calculate_perspective(ratio);
    calculate_view();
}

void get::camera::calculate_perspective(f64 ratio)
{
    _projection_matrix = glm::perspective(glm::radians(_settings.fov), ratio, _settings.near_clip, _settings.far_clip);
}

void get::camera::calculate_view()
{
    // pitch / yaw / roll
    _roll += glm::radians(10.f) * frame_time::delta_time();
    glm::quat rot = glm::vec3(0.0f, 0.0f, _roll);
    glm::vec3 pos = glm::vec3(0.0f, 0.0f, 1.0f); 

    _view_matrix = glm::translate(glm::mat4(1.0f), pos) * glm::mat4(rot);
    _view_matrix = glm::inverse(_view_matrix);
}

glm::mat4 get::camera::get_view_projection_matrix() 
{
    _view_projection_matrix = _projection_matrix * _view_matrix;
    return _view_projection_matrix;
}
