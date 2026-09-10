#version 450
    
#extension GL_EXT_nonuniform_qualifier : require

layout (location = 0) in vec3 _in_color;
layout (location = 1) in vec3 _in_normal;
layout (location = 2) in vec2 _in_uv;
layout (location = 3) in flat uint _in_texture_index;
layout (location = 4) in flat vec4 _in_material_base_color;
layout (location = 0) out vec4 _out_color;

layout(set = 0, binding = 0) uniform sampler2D textures[];

void main()
{
    vec3 normal = normalize(_in_normal);
    vec3 lightDirection = normalize(vec3(0, -1, -1));
    float d = max(dot(normal, -lightDirection), 0);
    vec4 texColor = texture(textures[_in_texture_index], _in_uv);

    // two-tone ambient light
    vec3 skyColor = vec3(0.15, 0.18, 0.25);
    vec3 groundColor = vec3(0.05, 0.03, 0.02);
    float t = normal.y * 0.5 + 0.5;
    vec3 hemiAmbient = mix(groundColor, skyColor, t);

    vec3 finalColor = _in_color * texColor.rgb * _in_material_base_color.rgb;
    vec3 litColor = finalColor * d + finalColor * hemiAmbient;
    
    _out_color = vec4(litColor, texColor.a);
}
