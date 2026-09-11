#pragma once
#include <string>
#include <volk.h>
#include <glm/matrix.hpp>
#include "shader.h"

namespace get
{
    class vk_pipeline
    {
    public:

        vk_pipeline(VkDevice device, const shader& shader, VkDescriptorSetLayout dsLayout);
        ~vk_pipeline();
        
        
        vk_pipeline(const vk_pipeline&) = delete;
        vk_pipeline(vk_pipeline&&) = delete;
        vk_pipeline& operator=(const vk_pipeline&) = delete;
        vk_pipeline& operator=(vk_pipeline&&) = delete;

        [[nodiscard]] VkPipeline get_pipeline() const;
        [[nodiscard]] VkPipelineLayout get_layout() const;

    private:

        const std::string _shader_entry_point;
        VkPipelineLayout _pipeline_layout;
        VkPipeline _pipeline;
        VkDevice _device;
    };
}
