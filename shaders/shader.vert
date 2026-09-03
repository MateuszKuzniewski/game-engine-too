#version 450

layout(location = 0) out vec3 fragColor;
layout(push_constant) uniform pushConstants 
{
    mat4 vpm;
} pc;

const vec3[3] verts = vec3[]
(
    vec3(0.0, -0.5, 0.0),
    vec3(-0.5, 0.5, 0.0),
    vec3(0.5,  0.5, 0.0)
);

const vec3[3] colors = vec3[]
(
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 1.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);


void main()
{
    gl_Position = pc.vpm * vec4(verts[gl_VertexIndex], 1.0);
    fragColor = colors[gl_VertexIndex];
}
