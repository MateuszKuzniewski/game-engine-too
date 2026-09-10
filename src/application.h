#pragma once
#include <memory>
#include <filesystem>
#include "window.h"
#include "glfw_context.h"
#include "vulkan_surface.h"
#include "vulkan_context.h"
#include "vulkan_queue_family.h"
#include "vulkan_physical_device.h"
#include "vulkan_device.h"
#include "vulkan_memory_allocator.h"
#include "vulkan_swapchain.h"
#include "vulkan_semaphore.h"
#include "vulkan_pipeline.h"
#include "depth_buffer.h"
#include "shader.h"
#include "command_pool.h"
#include "command_buffer.h"
#include "camera.h"
#include "render_data.h"
#include "tiny_gltf_v3.h"
#include "node_world.h"
#include "vk_descriptor_set.h"

class application
{
public:

    application();
    ~application();
    
    application(const application&) = delete;
    application(application&&) = delete;
    application& operator=(const application&) = delete;
    application& operator=(application&&) = delete;
    
    void load_data();
    void run();


private:
    
    // TO DO: move this out of here
    VkCommandBuffer start_transient_command_buffer();
    
    void submit_transient_command_buffer(VkCommandBuffer commandBuffer);
    void load_gltf(const std::string& filepath);
    void update_texture_descriptors() const;
    void create_indirect_buffers();   
    std::pair<u32, get::gpu_buffer> create_image(VkCommandBuffer commandBuffer, unsigned char* imageData, u32 width, u32 height, i32 channels);
    
    get::gpu_buffer create_buffer(VkBufferUsageFlags usage, size_t byteSize, bool mappable, VmaMemoryUsage memoryUsage);
    
    std::vector<get::image> load_images(const tg3_model& model, const std::filesystem::path& path);
    std::vector<u32> upload_images(const std::vector<get::image>& images);
    std::vector<u32> load_samplers(const tg3_model& model);
    std::vector<u32> load_textures(const tg3_model& model, const std::vector<u32>& images, const std::vector<u32>& samplers);
    std::vector<u32> load_materials(const tg3_model& model, const std::vector<u32>& textures);
    std::vector<u32> load_meshes(const tg3_model& model, const std::vector<u32>& materials);

    u32 import_node(get::node_world& nodeWorld, const tg3_model& model, i32 nodeIndex, u32 parentId, u32 prevSiblingId, std::vector<u32>& meshIds);
    u32 add_buffer(const get::gpu_buffer& buffer);
    ////////////////

    void render(int width, int height);

private:

    static constexpr u32 MAX_TEXTURES = 1024;
    
    u32 _vertex_buffer_id = 0;
    u32 _index_buffer_id = 0;
    u32 _material_buffer_id = 0;

    u32 _frame_index;
    u32 _max_frames_in_flight;
    u32 _fallback_image_id;
    u64 _next_signal_value;

    u32 _last_root_node_id = 0;
    u32 _root_node_id = 0;
    size_t _vert_offset = 0;
    size_t _id_offset = 0;
    

    bool _recreate_swapchain = false;
    
    std::vector<std::pair<get::node*, glm::mat4>> _node_render_stack;

    std::vector<get::frame_resource> _frame_resources;
    
    std::vector<get::gpu_image> _gpu_images;

    std::vector<get::vertex> _vertices;
    std::vector<u32> _indices;
    std::vector<get::gpu_buffer> _buffers;
    std::vector<VkSampler> _samplers;
    std::vector<get::texture> _textures;
    std::vector<get::material> _materials;
    std::vector<get::mesh> _meshes;

    std::unique_ptr<get::glfw_context> _glfw_context;
    std::unique_ptr<get::vulkan_context> _vulkan_context;
    std::unique_ptr<get::window> _window;
    std::unique_ptr<get::vulkan_surface> _surface;
    std::unique_ptr<get::vulkan_physical_device> _physical_device;
    std::unique_ptr<get::vulkan_queue_family> _queue_family;
    std::unique_ptr<get::vulkan_device> _vulkan_device;
    std::unique_ptr<get::vulkan_memory_allocator> _vma;
    std::unique_ptr<get::vulkan_swapchain> _swapchain;
    std::unique_ptr<get::depth_buffer> _depth_buffer;
    std::unique_ptr<get::shader> _shader;
    std::unique_ptr<get::vulkan_pipeline> _vulkan_pipeline;
    std::unique_ptr<get::vulkan_sempahore> _semaphore;
    std::unique_ptr<get::command_pool> _command_pool;
    std::unique_ptr<get::command_buffer> _command_buffer;
    std::unique_ptr<get::camera> _main_camera;
    std::unique_ptr<get::node_world> _node_world;
    std::unique_ptr<get::vk_descriptor_set> _descriptor_set;
};

