#pragma once
#include <volk.h>
#include <glm/matrix.hpp>
#include "shader.h"

namespace get
{
    class vk_pipeline
    {
    public:

        vk_pipeline(
                VkDevice device, 
                const shader& shader,
                const std::vector<VkDescriptorSetLayout>& dsLayout,
                VkFormat swapchainFormat, 
                VkFormat depthFormat,
                VkPipelineColorBlendAttachmentState attachState,
                VkPipelineDepthStencilStateCreateInfo depthStencilInfo,
                VkPipelineRasterizationStateCreateInfo rasterInfo);

        ~vk_pipeline();
        
        
        vk_pipeline(const vk_pipeline&) = delete;
        vk_pipeline(vk_pipeline&&) = delete;
        vk_pipeline& operator=(const vk_pipeline&) = delete;
        vk_pipeline& operator=(vk_pipeline&&) = delete;

        [[nodiscard]] VkPipeline get_pipeline() const;
        [[nodiscard]] VkPipelineLayout get_layout() const;
    
    private:

        VkResult create_layout(const std::vector<VkDescriptorSetLayout>& descriptorSetLayout);
        VkResult create_pipeline(
                const shader& shader, 
                VkFormat swapchainFormat,
                VkFormat depthFormat,
                VkPipelineColorBlendAttachmentState attachState,
                VkPipelineDepthStencilStateCreateInfo depthStencilInfo,
                VkPipelineRasterizationStateCreateInfo rasterInfo);

    private:

        VkPipelineLayout _pipeline_layout;
        VkPipeline _pipeline;
        VkDevice _device;
    };
}
