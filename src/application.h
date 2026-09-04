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


class application
{
public:

    application();
    ~application();
    
    application(const application&) = delete;
    application(application&&) = delete;
    application& operator=(const application&) = delete;
    application& operator=(application&&) = delete;

    void run();

private:

    void render(int width, int height);

private:

    u32 _frame_index;
    u32 _max_frames_in_flight;

    u64 _next_signal_value;
    
    bool _recreate_swapchain = false;

    std::vector<get::frame_resource> _frame_resources;
    
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

