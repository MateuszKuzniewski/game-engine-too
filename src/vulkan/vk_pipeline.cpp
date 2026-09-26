#include <stdexcept>
#include <vector>
#include "types.h"
#include "vk_pipeline.h"
#include "render_data.h"

get::vk_pipeline::vk_pipeline(
    VkDevice device, 
    const shader& shader,
    const std::vector<VkDescriptorSetLayout>& dsLayout,
    VkFormat swapchainFormat, 
    VkFormat depthFormat,
    VkPipelineColorBlendAttachmentState attachState,
    VkPipelineDepthStencilStateCreateInfo depthStencilInfo,
    VkPipelineRasterizationStateCreateInfo rasterInfo) : _device(device)
{
    if (create_layout(dsLayout) != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create vulkan pipeline");
    }

    if (create_pipeline(shader, swapchainFormat, depthFormat, attachState, depthStencilInfo, rasterInfo) != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create a new pipeline");
    }
}

get::vk_pipeline::~vk_pipeline()
{
    vkDestroyPipelineLayout(_device, _pipeline_layout, nullptr);
    vkDestroyPipeline(_device, _pipeline, nullptr);
}

VkResult get::vk_pipeline::create_layout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayout)
{
    VkPushConstantRange pushConstantRange
    {
        .stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
        .offset = 0,
        .size = sizeof(frame_constants)
    };

    VkPipelineLayoutCreateInfo pipelineLayoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO,
        .setLayoutCount = static_cast<u32>(descriptorSetLayout.size()),
        .pSetLayouts = descriptorSetLayout.data(),
        .pushConstantRangeCount = 1,
        .pPushConstantRanges = &pushConstantRange
    };

    return vkCreatePipelineLayout(_device, &pipelineLayoutInfo, nullptr, &_pipeline_layout);
}

VkResult get::vk_pipeline::create_pipeline(
        const shader& shader, 
        VkFormat swapchainFormat,
        VkFormat depthFormat,
        VkPipelineColorBlendAttachmentState attachState,
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo,
        VkPipelineRasterizationStateCreateInfo rasterInfo)
{
    auto vert = shader.compile(shader_type::VERT);
    auto frag = shader.compile(shader_type::FRAG);
    
    auto entryPoint = shader.get_entry_point();
    std::vector<VkPipelineShaderStageCreateInfo> shaderStages
    {
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_VERTEX_BIT,
            .module = vert,
            .pName = entryPoint.c_str()
        },
        {
            .sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO,
            .stage = VK_SHADER_STAGE_FRAGMENT_BIT,
            .module = frag,
            .pName = entryPoint.c_str()
        }
    };
    
    VkPipelineVertexInputStateCreateInfo vertInputInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO,
    };

    VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO,
        .topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST
    };

    VkPipelineViewportStateCreateInfo viewportInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO,
        .viewportCount = 1,
        .pViewports = nullptr,
        .scissorCount = 1,
        .pScissors = nullptr
    };

    
    VkPipelineMultisampleStateCreateInfo multiSampleInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO,
        .rasterizationSamples = VK_SAMPLE_COUNT_1_BIT
    };

   
    VkPipelineColorBlendStateCreateInfo blendInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO,
        .logicOpEnable = VK_FALSE,
        .logicOp = VK_LOGIC_OP_COPY,
        .attachmentCount = 1,
        .pAttachments = &attachState
    };

    std::vector<VkDynamicState> dynamicState
    {
        VK_DYNAMIC_STATE_VIEWPORT, 
        VK_DYNAMIC_STATE_SCISSOR,
    };

    VkPipelineDynamicStateCreateInfo dynamicStateInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO,
        .dynamicStateCount = static_cast<u32>(dynamicState.size()),
        .pDynamicStates = dynamicState.data()
    };

    VkPipelineRenderingCreateInfo renderInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &swapchainFormat,
        .depthAttachmentFormat = depthFormat
    };

    VkGraphicsPipelineCreateInfo pipelineInfo
    {
        .sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO,
        .pNext = &renderInfo,
        .stageCount = static_cast<u32>(shaderStages.size()),
        .pStages = shaderStages.data(),
        .pVertexInputState = &vertInputInfo,
        .pInputAssemblyState = &inputAssemblyInfo,
        .pViewportState = &viewportInfo,
        .pRasterizationState = &rasterInfo,
        .pMultisampleState = &multiSampleInfo,
        .pDepthStencilState = &depthStencilInfo,
        .pColorBlendState = &blendInfo,
        .pDynamicState = &dynamicStateInfo,
        .layout = _pipeline_layout,
        .renderPass = VK_NULL_HANDLE,
    };

    VkResult res = vkCreateGraphicsPipelines(_device, nullptr, 1, &pipelineInfo, nullptr, &_pipeline);

    vkDestroyShaderModule(_device, vert, nullptr);
    vkDestroyShaderModule(_device, frag, nullptr);

    return res;
}

VkPipeline get::vk_pipeline::get_pipeline() const
{
    return _pipeline;
}

VkPipelineLayout get::vk_pipeline::get_layout() const
{
    return _pipeline_layout;
}
