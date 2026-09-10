#pragma once

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/matrix_decompose.hpp>
#include "types.h"

namespace get
{
    struct node_data
    {
        u32 mesh_id;
        u32 parent_id;
        u32 next_sibling_id;
        u32 first_child_id;
    };

    class node
    {
    public:

        node_data& data() { return _data; };

        glm::vec3 get_translation() const { return _translation; }

        void set_translation(const glm::vec3& translation)
        {
            _translation = translation;
            _dirty = true;
        }

        glm::quat get_rotation() const { return _rotation; }

        void set_rotation(const glm::quat& rotation)
        {
            _rotation = rotation;
            _dirty = true;
        }

        glm::vec3 get_scale() const { return _scale; } 

        void set_scale(const glm::vec3 scale)
        {
            _scale = scale;
            _dirty = true;
        }

        glm::mat4 get_transform()
        {
            if (_dirty)
            {
                glm::mat4 matTranslate = glm::translate(glm::mat4(1), _translation);
                glm::mat4 matRotate = glm::mat4_cast(_rotation);
                glm::mat4 matScale = glm::scale(glm::mat4(1), _scale);
                _transform = matTranslate * matRotate * matScale;
                _dirty = false;
            }

            return _transform;
        }

        void set_transform(glm::mat4& transform)
        {
            glm::vec3 skew;
            glm::vec4 perspective;
            glm::decompose(transform, _scale, _rotation, _translation, skew, perspective);

            _transform = transform;
            _dirty = false;
        }

    private:

        node_data _data {};

        glm::vec3 _translation = glm::vec3(0,0,0);
        glm::vec3 _scale = glm::vec3(1,1,1);
        glm::quat _rotation = glm::quat(1,0,0,0);
        glm::mat4 _transform = glm::mat4(1);
        bool _dirty = true;
    };
}
