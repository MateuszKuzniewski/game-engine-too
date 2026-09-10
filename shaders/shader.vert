#version 450

#extension GL_EXT_buffer_reference : require
#extension GL_EXT_scalar_block_layout : require
#extension GL_EXT_shader_explicit_arithmetic_types_int64 : require

layout(push_constant, scalar) uniform frame_constants
{
    uint64_t vertex_buffer_address;
    uint64_t material_buffer_address;
    uint64_t render_item_buffer_address;
} frameConstants;

struct vertex
{
    vec3 position;
    vec3 color;
    vec3 normal;
    vec2 uv;
};

layout(buffer_reference, scalar) readonly buffer vertex_ptr
{
    vertex vertices[];
};

struct material
{
    vec4 base_color;
    uint color_texture_index;
};

layout(buffer_reference, scalar) readonly buffer material_ptr
{
    material materials[];
};

struct render_item
{
    mat4x4 wvp;
    mat4x4 world_matrix;
    uint material_index;
};

layout(buffer_reference, scalar) readonly buffer render_item_ptr
{
    render_item render_items[];
};

layout (location = 0) out vec3 _out_color;
layout (location = 1) out vec3 _out_normal;
layout (location = 2) out vec2 _out_uv;
layout (location = 3) out flat uint _out_texture_index;
layout (location = 4) out flat vec4 _out_material_base_color;

void main()
{
    vertex_ptr vertexBuffer = vertex_ptr(frameConstants.vertex_buffer_address);
    vertex v = vertexBuffer.vertices[gl_VertexIndex];

    render_item_ptr renderItemBuffer = render_item_ptr(frameConstants.render_item_buffer_address);
    render_item renderItem = renderItemBuffer.render_items[gl_InstanceIndex];

    material_ptr materialBuffer = material_ptr(frameConstants.material_buffer_address);
    material m = materialBuffer.materials[renderItem.material_index];

    gl_Position = renderItem.wvp * vec4(v.position, 1.0);
    _out_color = v.color;
    _out_normal = mat3x3(transpose(inverse(renderItem.world_matrix))) * v.normal;
    _out_uv = v.uv;
    _out_texture_index = m.color_texture_index;
    _out_material_base_color = m.base_color;
}
