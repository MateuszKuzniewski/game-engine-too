#pragma once
#include <string>
#include <volk.h>
#include "shader.h"

namespace get
{
    class vulkan_pipeline
    {
    public:

        vulkan_pipeline(VkDevice device, const shader& shader);
        ~vulkan_pipeline();
        
        [[nodiscard]] VkPipeline get_pipeline() const;

    private:

        const std::string _shader_entry_point;
        VkPipelineLayout _pipeline_layout;
        VkPipeline _pipeline;
        VkDevice _device;
    };
}
