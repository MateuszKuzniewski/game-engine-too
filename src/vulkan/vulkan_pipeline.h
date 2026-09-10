#pragma once
#include <string>
#include <volk.h>
#include <glm/matrix.hpp>
#include "shader.h"

namespace get
{
    class vulkan_pipeline
    {
    public:

        vulkan_pipeline(VkDevice device, const shader& shader, VkDescriptorSetLayout dsLayout);
        ~vulkan_pipeline();
        
        
        vulkan_pipeline(const vulkan_pipeline&) = delete;
        vulkan_pipeline(vulkan_pipeline&&) = delete;
        vulkan_pipeline& operator=(const vulkan_pipeline&) = delete;
        vulkan_pipeline& operator=(vulkan_pipeline&&) = delete;

        [[nodiscard]] VkPipeline get_pipeline() const;
        [[nodiscard]] VkPipelineLayout get_layout() const;

    private:

        const std::string _shader_entry_point;
        VkPipelineLayout _pipeline_layout;
        VkPipeline _pipeline;
        VkDevice _device;
    };
}
