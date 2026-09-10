#pragma once
#include <volk.h>
#include <vector>
#include "types.h"
#include "render_data.h"

namespace get
{
    class vk_descriptor_set
    {
    public:

        vk_descriptor_set(VkDevice device, const u32 maxTextures);
        ~vk_descriptor_set() = default;

        vk_descriptor_set(const vk_descriptor_set&) = delete;
        vk_descriptor_set(vk_descriptor_set&&) = delete;
        vk_descriptor_set& operator=(const vk_descriptor_set&) = delete;
        vk_descriptor_set& operator=(vk_descriptor_set&&) = delete;
        
        void update_texture_descriptors(
                const std::vector<get::texture>& textures,
                const std::vector<VkSampler>& samplers,
                const std::vector<get::gpu_image>& images) const;

        [[nodiscard]] VkDescriptorSet get_global_descriptor_set() const;
        [[nodiscard]] VkDescriptorSetLayout get_descriptor_set_layout() const;
    
        
    private:

        VkDescriptorSet _global_descriptor_set;
        VkDescriptorSetLayout _global_descriptor_set_layout;
        VkDescriptorPool _pool;
        VkDevice _device;
    };
}
