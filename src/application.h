#pragma once
#include <memory>
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
#include "frame_resources.h"
#include "command_pool.h"
#include "command_buffer.h"
#include "camera.h"
#include "render_data.h"

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
    std::pair<u32, get::gpu_buffer> create_image(VkCommandBuffer commandBuffer, unsigned char* imageData, u32 width, u32 height, i32 channels);
    get::gpu_buffer create_buffer(VkBufferUsageFlags usage, size_t byteSize, bool mappable, VmaMemoryUsage memoryUsage);
    void map_copy_buffer_data(const get::gpu_buffer& buffer, size_t bufferOffset, void* data, size_t byteSize);
    void load_gltf(const std::string& filepath);
    ////////////////

    void render(int width, int height);

private:

    u32 _frame_index;
    u32 _max_frames_in_flight;
    u32 _fallback_image_id;
    u64 _next_signal_value;
    
    bool _recreate_swapchain = false;

    std::vector<get::frame_resource> _frame_resources;
    
    std::vector<get::vertex> _vertices;
    std::vector<u32> _indices;

    std::vector<get::gpu_image> _gpu_images;
    std::vector<VkSampler> _samplers;
    std::vector<get::texture> _textures;

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
};

