#pragma once
#include <glm/glm.hpp>

namespace get
{
    namespace world
    {
        inline constexpr glm::vec3 up       { 0.0f, 1.0f, 0.0f };
        inline constexpr glm::vec3 down     { 0.0f, -1.0f, 0.0f };
        inline constexpr glm::vec3 right    { 1.0f, 0.0f, 0.0f };
        inline constexpr glm::vec3 left     { -1.0f, 0.0f, 0.0f };
        inline constexpr glm::vec3 forward  { 0.0f, 0.0f, -1.0f };
        inline constexpr glm::vec3 backward { 0.0f, 0.0f, 1.0f };
        inline constexpr glm::vec3 zero     { 0.0f, 0.0f, 0.0f };
        inline constexpr glm::vec3 one      { 1.0f, 1.0f, 1.0f };
    }
}
