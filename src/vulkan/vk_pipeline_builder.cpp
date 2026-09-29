#include "vk_pipeline_builder.h"

get::vk_pipeline_builder::vk_pipeline_builder(VkDevice device) : _device(device)
{

}

get::vk_pipeline_builder::~vk_pipeline_builder()
{

}

get::vk_pipeline_builder& get::vk_pipeline_builder::reset()
{
    _color_format = VK_FORMAT_UNDEFINED;
    _depth_format = VK_FORMAT_UNDEFINED;

    _descriptor_set_layouts.clear();

    _color_blend_attachment.blendEnable = VK_FALSE;
    _color_blend_attachment.colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
            VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT;

    _depth_stencil_info.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
    _depth_stencil_info.depthTestEnable = VK_TRUE;
    _depth_stencil_info.depthWriteEnable = VK_TRUE;
    _depth_stencil_info.depthCompareOp = VK_COMPARE_OP_LESS;


    _raster_info.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
    _raster_info.polygonMode = VK_POLYGON_MODE_FILL;
    _raster_info.cullMode = VK_CULL_MODE_BACK_BIT;
    _raster_info.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
    _raster_info.lineWidth = 1.0f;

    return *this;
}   

get::vk_pipeline_builder& get::vk_pipeline_builder::set_shader(const shader& shader)
{
    _shader = &shader;
    return *this;
}

get::vk_pipeline_builder& get::vk_pipeline_builder::set_formats(VkFormat colorFormat, VkFormat depthFormat)
{
    _color_format = colorFormat;
    _depth_format = depthFormat;
    return *this;
}

get::vk_pipeline_builder& get::vk_pipeline_builder::add_descriptor_set_layout(VkDescriptorSetLayout layout)
{
    _descriptor_set_layouts.push_back(layout);
    return *this;
}


get::vk_pipeline_builder& get::vk_pipeline_builder::enable_blending_alpha()
{
    _color_blend_attachment.blendEnable = VK_TRUE;
    _color_blend_attachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
    _color_blend_attachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
    _color_blend_attachment.colorBlendOp = VK_BLEND_OP_ADD;
    _color_blend_attachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
    _color_blend_attachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
    _color_blend_attachment.alphaBlendOp = VK_BLEND_OP_ADD;
    return *this;
}

get::vk_pipeline_builder& get::vk_pipeline_builder::set_depth_test(bool testEnable, bool writeEnable, VkCompareOp op)
{
    _depth_stencil_info.depthTestEnable = testEnable;
    _depth_stencil_info.depthWriteEnable = writeEnable;
    _depth_stencil_info.depthCompareOp = op;
    return *this;
}

get::vk_pipeline_builder& get::vk_pipeline_builder::disable_depth_test()
{
    _depth_stencil_info.depthTestEnable = VK_FALSE;
    _depth_stencil_info.depthWriteEnable = VK_FALSE;
    return *this;
}


get::vk_pipeline_builder& get::vk_pipeline_builder::set_cull_mode(VkCullModeFlagBits flags, VkFrontFace face)
{
    _raster_info.cullMode = flags;
    _raster_info.frontFace = face;
    return *this;
}

std::unique_ptr<get::vk_pipeline> get::vk_pipeline_builder::build() const
{
    return std::make_unique<vk_pipeline>(
            _device,
            *_shader,
            _descriptor_set_layouts,
            _color_format,
            _depth_format,
            _color_blend_attachment,
            _depth_stencil_info,
            _raster_info);
}
