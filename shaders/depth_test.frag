#version 450

#extension GL_EXT_nonuniform_qualifier : require

layout (location = 2) in vec2 _in_uv;
layout (location = 3) in flat uint _in_texture_index;

layout(set = 0, binding = 0) uniform sampler2D textures[];


void main()
{
    vec4 texColor = texture(textures[_in_texture_index], _in_uv);
    if (texColor.a < 0.1)
    {
        discard;
    }
}
