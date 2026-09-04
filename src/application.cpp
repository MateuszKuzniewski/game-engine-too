#include <memory>
#include <print>
#include "application.h"
#include "directories.h"
#include "stb_image.h"
#include "frame_time.h"

application::application() : _frame_index(0), _max_frames_in_flight(2), _next_signal_value(2), _frame_resources(2)
{
    std::println("{0}", "SYSTEM: Application was created");
    std::println("{0}{1}", "SYSTEM: Project path is set to: ", get::directories::project_path());
    std::println("{0}{1}", "SYSTEM: Shader path is set to: ", get::directories::shader_path());

    get::window_settings settings
    {
        .width = 1280,
        .height = 720, 
        .title = "Game Engine Too"
    };

    get::camera_settings cameraSettings
    {
        .fov = 60.f,
        .near_clip = 0.1f,
        .far_clip = 10000.f,
    };
    
    _glfw_context =     std::make_unique<get::glfw_context>();
    _vulkan_context =   std::make_unique<get::vulkan_context>(*_glfw_context, settings.title);
    _window =           std::make_unique<get::window>(settings);
    _surface =          std::make_unique<get::vulkan_surface>(*_window, _vulkan_context->get_instance());
    _physical_device =  std::make_unique<get::vulkan_physical_device>(_vulkan_context->get_instance());
    _queue_family =     std::make_unique<get::vulkan_queue_family>(_physical_device->get_device(), _surface->get_surface());
    _vulkan_device =    std::make_unique<get::vulkan_device>(_physical_device->get_device(), _queue_family->get_queue_family_id());
    _vma =              std::make_unique<get::vulkan_memory_allocator>(
                            _vulkan_context->get_instance(), 
                            _physical_device->get_device(), 
                            _vulkan_device->get_device());

    _swapchain =        std::make_unique<get::vulkan_swapchain>(
                            _vulkan_device->get_device(), 
                            _physical_device->get_device(),
                            _surface->get_surface());

    _swapchain->create(settings.width, settings.height);

    _depth_buffer =     std::make_unique<get::depth_buffer>(
                            _vulkan_device->get_device(), 
                            _vma->get_allocator(), 
                            settings.width, 
                            settings.height);

    _depth_buffer->create(settings.width, settings.height);

    _shader =           std::make_unique<get::shader>(
                            _vulkan_device->get_device(),
                            "shader.vert",
                            "shader.frag");

    _vulkan_pipeline =  std::make_unique<get::vulkan_pipeline>( _vulkan_device->get_device(), *_shader);

    _semaphore = std::make_unique<get::vulkan_sempahore>(_vulkan_device->get_device(), _frame_resources, _max_frames_in_flight);

    _command_pool = std::make_unique<get::command_pool>(_vulkan_device->get_device(), _queue_family->get_queue_family_id(), _frame_resources);

    _command_buffer = std::make_unique<get::command_buffer>(_vulkan_device->get_device(), _frame_resources);

    _main_camera = std::make_unique<get::camera>(settings.width, settings.height, cameraSettings);

}

application::~application()
{
    vkDeviceWaitIdle(_vulkan_device->get_device());

    for (auto& res : _frame_resources)
    {
        vkDestroySemaphore(_vulkan_device->get_device(), res.image_acquired_semaphore, nullptr);
        vkDestroyCommandPool(_vulkan_device->get_device(), res.command_pool, nullptr);
    }
    std::println("{0}", "SYSTEM: Application was destroyed");
}

void application::run()
{
    std::println("{0}", "SYSTEM: Application is running");

    int currentWidth = 0;
    int currentHeight = 0;
    int lastWidth = 0;
    int lastHeight = 0;
    auto window = _window->get_current_window();

    while (!glfwWindowShouldClose(window))
    {
        get::frame_time::update();
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);

        while (currentWidth == 0 || currentHeight == 0)
        {
            glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
            glfwWaitEvents();
        }
        
        if (currentWidth != lastWidth || currentHeight != lastHeight)
        {
            _recreate_swapchain = true;
            lastWidth = currentWidth;
            lastHeight = currentHeight;
        }
        
        _main_camera->update(currentWidth, currentHeight);
        render(currentWidth, currentHeight);
        glfwPollEvents();
    }
}

void application::render(int width, int height)
{
    if (_recreate_swapchain)
    {
        vkDeviceWaitIdle(_vulkan_device->get_device());
        _swapchain->destroy();
        _swapchain->create(width, height);
        _depth_buffer->destroy();
        _depth_buffer->create(width, height);

        _recreate_swapchain = false;
    }

    const u32 frameResIndex = _frame_index++ % _max_frames_in_flight;
    const u64 signalValue = ++_next_signal_value;
    const u64 waitValue = (signalValue > _max_frames_in_flight) ? (signalValue - _max_frames_in_flight) : 0;

    auto semaphore = _semaphore->get_semaphore();
    VkSemaphoreWaitInfo waitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = &semaphore,
        .pValues = &waitValue,
    };

    vkWaitSemaphores(_vulkan_device->get_device(), &waitInfo, UINT64_MAX);
    
    // start working on the frame
    auto& resource = _frame_resources[frameResIndex];
    vkResetCommandPool(_vulkan_device->get_device(), resource.command_pool, 0);

    VkSemaphore imageAcquireSemaphore = _frame_resources[frameResIndex].image_acquired_semaphore;

    u32 imageIndex = 0;
    VkResult res = vkAcquireNextImageKHR(_vulkan_device->get_device(),
                                            _swapchain->get_swapchain(),
                                            UINT64_MAX,
                                            imageAcquireSemaphore,
                                            VK_NULL_HANDLE, &imageIndex);

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        _recreate_swapchain = true;
        return;
    }
    else if (res == VK_SUBOPTIMAL_KHR)
    {
        _recreate_swapchain = true;
    }

    // begin recording commands
    VkCommandBufferBeginInfo commandBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(resource.command_buffer, &commandBeginInfo);
    
    std::vector<VkImageMemoryBarrier2> layoutBarriers
    {
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = _swapchain->get_swapchain_images()[imageIndex],
            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        },
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .image = _depth_buffer->get_image(),
            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        }
    };

    VkDependencyInfo dependencyInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<u32>(layoutBarriers.size()),
        .pImageMemoryBarriers = layoutBarriers.data(),
    };
    
    vkCmdPipelineBarrier2(resource.command_buffer, &dependencyInfo);
    
    // setup attachments
    VkRenderingAttachmentInfo colorAttachInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = _swapchain->get_swapchain_image_views()[imageIndex],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue 
        { 
            .color = {{ 0.01f, 0.01f, 0.01f, 1 }}
        }
    };

    VkRenderingAttachmentInfo depthAttachInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = _depth_buffer->get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue 
        {
            .depthStencil { 1.0f, 0 }
        }
    };

    VkRenderingInfo renderingInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea 
        {
            .offset { .x = 0, .y = 0 },
            .extent
            {
                .width = static_cast<u32>(width),
                .height = static_cast<u32>(height)
            }
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachInfo,
        .pDepthAttachment = &depthAttachInfo
    };

    // begin dynamic rendering
    vkCmdBeginRendering(resource.command_buffer, &renderingInfo);
    {
        VkViewport viewport
        {
            .x = 0, .y = 0,
            .width = static_cast<f32>(width),
            .height = static_cast<f32>(height),
        };
        vkCmdSetViewport(resource.command_buffer, 0, 1, &viewport);

        VkRect2D scissor
        {
            .offset { .x = 0, .y = 0},
            .extent 
            { 
                .width = static_cast<u32>(width),
                .height = static_cast<u32>(height),
            }
        };

        get::push_constant_data pushData { .vpm = _main_camera->get_view_projection_matrix() };
        vkCmdPushConstants(
                resource.command_buffer,
                _vulkan_pipeline->get_layout(),
                VK_SHADER_STAGE_VERTEX_BIT, 
                0,
                static_cast<u32>(sizeof(get::push_constant_data)),
                &pushData);

        vkCmdSetScissor(resource.command_buffer, 0, 1, &scissor);
        vkCmdBindPipeline(resource.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vulkan_pipeline->get_pipeline());
        vkCmdDraw(resource.command_buffer, 3, 1, 0, 0);
    }
    vkCmdEndRendering(resource.command_buffer);
    
    // change memory layout of the swapchain to display the image
    VkImageMemoryBarrier2 presentLayoutBarrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = _swapchain->get_swapchain_images()[imageIndex],
        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    VkDependencyInfo presentDependencyInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &presentLayoutBarrier
    };

    vkCmdPipelineBarrier2(resource.command_buffer, &presentDependencyInfo);
    vkEndCommandBuffer(resource.command_buffer);
    
    // ensure swapchain image is available to start color output
    VkSemaphoreSubmitInfo imageAcquireWaitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = imageAcquireSemaphore,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    std::vector<VkSemaphoreSubmitInfo> semaphoreSignals
    {
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = _swapchain->get_render_complete_semaphores()[imageIndex],
            .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
        },
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = _semaphore->get_semaphore(),
            .value = signalValue,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
        }
    };

    VkCommandBufferSubmitInfo cmdSubmitInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = resource.command_buffer,
    };

    VkSubmitInfo2 submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &imageAcquireWaitInfo,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdSubmitInfo,
        .signalSemaphoreInfoCount = static_cast<u32>(semaphoreSignals.size()),
        .pSignalSemaphoreInfos = semaphoreSignals.data()
    };

    vkQueueSubmit2(_vulkan_device->get_queue(), 1, &submitInfo, VK_NULL_HANDLE);

    auto swapchain = _swapchain->get_swapchain();
    VkPresentInfoKHR presentInfo
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &_swapchain->get_render_complete_semaphores()[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };

    vkQueuePresentKHR(_vulkan_device->get_queue(), &presentInfo);
}

void application::shutdown()
{
    std::println("{0}", "SYSTEM: Application was closed");
} 
