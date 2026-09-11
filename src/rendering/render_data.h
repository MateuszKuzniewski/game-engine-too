#pragma once
#include <volk.h>
#include <glm/glm.hpp>
#include <string>
#include <vector>
#include "vk_mem_alloc.h"
#include "types.h"

namespace get
{
    struct vertex
    {
        glm::vec3 position = glm::vec3(0.0f);
        glm::vec3 color = glm::vec3(0.0f);
        glm::vec3 normal = glm::vec3(1.0f);
        glm::vec2 uv = glm::vec2(0.0f);
    };

    struct image
    {
        i32 width;
        i32 height;
        i32 channels;
        unsigned char *data;
    };

    struct gpu_buffer
    {
        VkBuffer buffer;
        u64 device_adress;
        VmaAllocation allocation;
    };

    struct gpu_image
    {
        VkImage image;
        VkImageView image_view;
        VmaAllocation allocation;
    };

    struct texture
    {
        u32 image_id;
        u32 sampler_id;
    };

    struct material
    {
        glm::vec4 base_color;
        u32 texture_id;
    };

    struct sub_mesh
    {
        size_t vertex_start;
        size_t vertex_count;
        size_t index_start;
        size_t index_count;
        u32 material_id;
    };

    struct mesh
    {
        std::string name;
        std::vector<sub_mesh> sub_meshes;
    };

    struct frame_constants
    {
        u64 vertex_buffer_address;
        u64 material_buffer_address;
        u64 render_items_buffer_address;
    };

    struct render_item 
    {
        glm::mat4 wvp;
        glm::mat4 world_matrix;
        u32 material_index;
    };

    struct frame_resource 
    {
        VkCommandPool command_pool = nullptr;
        VkCommandBuffer command_buffer = nullptr;
        VkSemaphore image_acquired_semaphore = nullptr;
        VkDescriptorSet descriptor_set = nullptr;
        get::gpu_buffer indirect_draw_buffer;
        get::gpu_buffer render_item_buffer;
        VkDrawIndexedIndirectCommand* indirect_draw_ptr = nullptr;
        render_item* render_item_ptr = nullptr;
    };
}
