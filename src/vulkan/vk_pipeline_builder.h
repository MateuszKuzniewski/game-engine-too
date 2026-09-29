#pragma once 
#include <volk.h>
#include <memory>
#include <vector>
#include "shader.h"
#include "vk_pipeline.h"

namespace get
{
    class vk_pipeline_builder
    {
    public:

        vk_pipeline_builder(VkDevice device);

        ~vk_pipeline_builder();
        
        vk_pipeline_builder& reset();
        vk_pipeline_builder& set_shader(const shader& shader);
        vk_pipeline_builder& set_formats(VkFormat colorFormat, VkFormat depthFormat);
        vk_pipeline_builder& add_descriptor_set_layout(VkDescriptorSetLayout layout);
        vk_pipeline_builder& enable_blending_alpha();
        vk_pipeline_builder& set_depth_test(bool testEnable, bool writeEnable, VkCompareOp op = VK_COMPARE_OP_LESS);
        vk_pipeline_builder& disable_depth_test();
        vk_pipeline_builder& set_cull_mode(VkCullModeFlagBits flags, VkFrontFace face);

        [[nodiscard]] std::unique_ptr<vk_pipeline> build() const;

    private:
        
        VkDevice _device;
        const shader* _shader;
        VkFormat _color_format = VK_FORMAT_UNDEFINED;
        VkFormat _depth_format = VK_FORMAT_UNDEFINED;

        std::vector<VkDescriptorSetLayout> _descriptor_set_layouts;

        VkPipelineColorBlendAttachmentState _color_blend_attachment
        {
            .blendEnable = VK_FALSE,
            .colorWriteMask = VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT |
                VK_COLOR_COMPONENT_B_BIT | VK_COLOR_COMPONENT_A_BIT
        };

        VkPipelineDepthStencilStateCreateInfo _depth_stencil_info
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO,
            .depthTestEnable = VK_TRUE,
            .depthWriteEnable = VK_TRUE,
            .depthCompareOp = VK_COMPARE_OP_LESS,
        };

        VkPipelineRasterizationStateCreateInfo _raster_info
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO,
            .polygonMode = VK_POLYGON_MODE_FILL,
            .cullMode = VK_CULL_MODE_BACK_BIT,
            .frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE,
            .lineWidth = 1.0f
        };
    };
}
