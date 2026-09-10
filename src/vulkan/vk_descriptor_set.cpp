#include "vk_descriptor_set.h"
#include <array>
#include <stdexcept>

get::vk_descriptor_set::vk_descriptor_set(VkDevice device, const u32 maxTextures) : _device(device)
{
    std::array<VkDescriptorPoolSize, 1> poolSizes
    {
        VkDescriptorPoolSize 
        {
            .type = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = maxTextures
        }
    };

    VkDescriptorPoolCreateInfo poolInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO,
        .flags = VK_DESCRIPTOR_POOL_CREATE_UPDATE_AFTER_BIND_BIT,
        .maxSets = 1,
        .poolSizeCount = poolSizes.size(),
        .pPoolSizes = poolSizes.data()
    };

    VkResult res = vkCreateDescriptorPool(_device, &poolInfo, nullptr, &_pool);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create descriptor set pool");
    }

    std::array<VkDescriptorSetLayoutBinding, 1> bindings
    {
        VkDescriptorSetLayoutBinding
        {
            .binding = 0,
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .descriptorCount = maxTextures,
            .stageFlags = VK_SHADER_STAGE_FRAGMENT_BIT
        }
    };

    std::array<VkDescriptorBindingFlags, 1> flags;
    flags[0] = VK_DESCRIPTOR_BINDING_PARTIALLY_BOUND_BIT | VK_DESCRIPTOR_BINDING_UPDATE_AFTER_BIND_BIT;

    VkDescriptorSetLayoutBindingFlagsCreateInfo flagsInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_BINDING_FLAGS_CREATE_INFO,
        .bindingCount = flags.size(),
        .pBindingFlags = flags.data()
    };

    VkDescriptorSetLayoutCreateInfo layoutInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_LAYOUT_CREATE_INFO,
        .pNext = &flagsInfo,
        .flags = VK_DESCRIPTOR_SET_LAYOUT_CREATE_UPDATE_AFTER_BIND_POOL_BIT,
        .bindingCount = bindings.size(),
        .pBindings = bindings.data(),
    };

    res = vkCreateDescriptorSetLayout(_device, &layoutInfo, nullptr, &_global_descriptor_set_layout);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to created descriptor set layout");
    }

    VkDescriptorSetAllocateInfo descriptorSetAllocInfo
    {
        .sType = VK_STRUCTURE_TYPE_DESCRIPTOR_SET_ALLOCATE_INFO,
        .descriptorPool = _pool,
        .descriptorSetCount = 1,
        .pSetLayouts = &_global_descriptor_set_layout
    };

    res = vkAllocateDescriptorSets(_device, &descriptorSetAllocInfo, &_global_descriptor_set);

    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to allocate descriptor set");
    }
}

void get::vk_descriptor_set::update_texture_descriptors( 
                const std::vector<get::texture>& textures,
                const std::vector<VkSampler>& samplers,
                const std::vector<get::gpu_image>& images) const
{
    std::vector<VkDescriptorImageInfo> imageDescriptors;
    imageDescriptors.reserve(textures.size());

    for (const auto& texture : textures)
    {
        imageDescriptors.push_back(
                {
                    .sampler = samplers[texture.sampler_id - 1],
                    .imageView = images[texture.image_id - 1].image_view,
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                });

        VkWriteDescriptorSet descSetWrite 
        {
            .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
            .dstSet = _global_descriptor_set,
            .dstBinding = 0,
            .dstArrayElement = 0,
            .descriptorCount = static_cast<u32>(imageDescriptors.size()),
            .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
            .pImageInfo = imageDescriptors.data()
        };

        vkUpdateDescriptorSets(_device, 1, &descSetWrite, 0, nullptr);
    }
}

VkDescriptorSet get::vk_descriptor_set::get_global_descriptor_set() const
{
    return _global_descriptor_set; 
}

VkDescriptorSetLayout get::vk_descriptor_set::get_descriptor_set_layout() const
{
    return _global_descriptor_set_layout;
}

