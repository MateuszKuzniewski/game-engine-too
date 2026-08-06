#version 450


const vec3[3] verts = vec3[]
(
    vec3(0.0, -0.5, 0.0),
    vec3(-0.5, 0.5, 0.0),
    vec3(0.5, 0.5, 0.0)
);

const vec3[3] colors = vec3[]
(
    vec3(1.0, 0.0, 0.0),
    vec3(1.0, 0.0, 0.0),
    vec3(0.0, 0.0, 1.0)
);

layout(location = 0) out vec3 fragColor;

void main()
{
    gl_Position = vec4(verts[gl_VertexIndex], 1.0);
    fragColor = colors[gl_VertexIndex];
}
