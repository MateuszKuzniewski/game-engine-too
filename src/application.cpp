#include <memory>
#include <cstring>
#include <print>
#include "application.h"
#include "directories.h"
#include "stb_image.h"
#include "frame_time.h"
#include "stb_image.h"

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

    _semaphore =        std::make_unique<get::vulkan_sempahore>(_vulkan_device->get_device(), _frame_resources, _max_frames_in_flight);

    _command_pool =     std::make_unique<get::command_pool>(_vulkan_device->get_device(), _queue_family->get_queue_family_id(), _frame_resources);

    _command_buffer =   std::make_unique<get::command_buffer>(_vulkan_device->get_device(), _frame_resources);

    _main_camera =      std::make_unique<get::camera>(settings.width, settings.height, cameraSettings);

}

application::~application()
{
    vkDeviceWaitIdle(_vulkan_device->get_device());

    for (auto& res : _frame_resources)
    {
        vkDestroySemaphore(_vulkan_device->get_device(), res.image_acquired_semaphore, nullptr);
        vkDestroyCommandPool(_vulkan_device->get_device(), res.command_pool, nullptr);
    }

    for (auto& img : _gpu_images)
    {
        vkDestroyImageView(_vulkan_device->get_device(), img.image_view, nullptr);
        vkDestroyImage(_vulkan_device->get_device(), img.image, nullptr);
        vmaFreeMemory(_vma->get_allocator(), img.allocation);
    }

    _gpu_images.clear();
    _vertices.clear();
    _indices.clear();

    std::println("{0}", "SYSTEM: Application was destroyed");
}

void application::load_data()
{
    constexpr size_t vertexBufferBytes = static_cast<size_t>(64 * 1024 * 1024); // vertex buffer size
    constexpr size_t indexBufferBytes = static_cast<size_t>(32 * 1024 * 1024); // index buffer size
    constexpr size_t totalVerts = vertexBufferBytes / sizeof(get::vertex);
    constexpr size_t totalIndices = indexBufferBytes / sizeof(u32);
    _vertices.resize(totalVerts);
    _indices.resize(totalIndices);

    u32 whitePixelData = 0xFFFFFFFF; // RGBA
    get::image fallbackTextureColor
    {
        .width = 1,
        .height = 1,
        .channels = 4,
        .data = reinterpret_cast<unsigned char*>(&whitePixelData)
    };

    VkCommandBuffer imageBuffer = start_transient_command_buffer();
    auto [whiteImageId, whiteStagingBuffer] = create_image(imageBuffer, fallbackTextureColor.data, fallbackTextureColor.width, fallbackTextureColor.height, 4);

    _fallback_image_id = whiteImageId;

    submit_transient_command_buffer(imageBuffer);

    vmaDestroyBuffer(_vma->get_allocator(), whiteStagingBuffer.buffer, whiteStagingBuffer.allocation);

    VkSamplerCreateInfo samplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .compareEnable = VK_FALSE
    };

    VkSampler sampler = nullptr;

    VkResult res = vkCreateSampler(_vulkan_device->get_device(), &samplerInfo, nullptr, &sampler);

    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create sampler");
    }

    _samplers.push_back(sampler);
    u32 fallbackSamplerID = _samplers.size();
    _textures.push_back(get::texture { .image_id = whiteImageId, .sampler_id = fallbackSamplerID });

}

void application::submit_transient_command_buffer(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    vkQueueSubmit(_vulkan_device->get_queue(), 1, &submitInfo, nullptr);
    vkQueueWaitIdle(_vulkan_device->get_queue());
    vkFreeCommandBuffers(_vulkan_device->get_device(), _command_pool->get_transient_command_pool(), 1, &commandBuffer);
}

std::pair<u32, get::gpu_buffer> application::create_image(VkCommandBuffer commandBuffer, unsigned char* imageData, u32 width, u32 height, i32 channels)
{
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageCreateInfo imageInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = imageFormat,
        .extent = { .width = width, .height = height, .depth = 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_AUTO };
    get::gpu_image gpuImage {};
    
    VkResult res = vmaCreateImage(_vma->get_allocator(), &imageInfo, &allocInfo, &gpuImage.image, &gpuImage.allocation, nullptr);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create image");
        return { 0, get::gpu_buffer{}};
    }

    VkImageViewCreateInfo imageViewInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = gpuImage.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = imageFormat,
        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };

    res = vkCreateImageView(_vulkan_device->get_device(), &imageViewInfo, nullptr, &gpuImage.image_view);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create image view");
    }

    VkImageMemoryBarrier2 transferBarrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .image = gpuImage.image,
        .subresourceRange
        {   
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    VkDependencyInfo transferDepInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &transferBarrier
    };

    vkCmdPipelineBarrier2(commandBuffer, &transferDepInfo);

    const size_t byteSize = width * height * channels;
    get::gpu_buffer stageBuffer = create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, byteSize, true, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    mapCopyBufferData(stageBuffer, 0, imageData, byteSize);

    VkBufferImageCopy bufferImageCopy
    {
        .imageSubresource 
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageExtent 
        {
            .width = width,
            .height = height,
            .depth = 1
        }
    };

    vkCmdCopyBufferToImage(commandBuffer, stageBuffer.buffer, gpuImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
    VkImageMemoryBarrier2 shaderReadBarrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
        .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .image = gpuImage.image,
        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    VkDependencyInfo shaderReadDepInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &shaderReadBarrier
    };

    vkCmdPipelineBarrier2(commandBuffer, &shaderReadDepInfo);
    
    _gpu_images.push_back(gpuImage);
    const u32 imageID = _gpu_images.size();
    return { imageID, stageBuffer} ;
    
}

void application::mapCopyBufferData(const get::gpu_buffer& buffer, size_t bufferOffset, void* data, size_t byteSize)
{
    void* bufferPtr = nullptr;
    if (vmaMapMemory(_vma->get_allocator(), buffer.allocation, &bufferPtr) != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Unable to map buffer memory");
        return;
    }

    std::memcpy(static_cast<char*>(bufferPtr) + bufferOffset, data, byteSize);
    vmaUnmapMemory(_vma->get_allocator(), buffer.allocation);
}

get::gpu_buffer application::create_buffer(VkBufferUsageFlags usage, size_t byteSize, bool mappable, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = byteSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo
    {
        .flags = mappable ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT: 0u,
        .usage = memoryUsage,
    };

    get::gpu_buffer gpuBuffer {};
    VkResult res = vmaCreateBuffer(_vma->get_allocator(), &bufferInfo, &allocInfo, &gpuBuffer.buffer, &gpuBuffer.allocation, nullptr);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create buffer");
        return get::gpu_buffer{};
    }

    // if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    // {
    //     VkBufferDeviceAddressInfo vertBdaInfo
    //     {
    //         .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
    //         .buffer = gpuBuffer.buffer
    //     };
    //
    //     gpuBuffer.device_adress = vkGetBufferDeviceAddress(_vulkan_device->get_device(), &vertBdaInfo);
    // }

    return gpuBuffer;


}

VkCommandBuffer application::start_transient_command_buffer()
{
    VkCommandBufferAllocateInfo allocInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _command_pool->get_transient_command_pool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer = nullptr;
    VkResult res = vkAllocateCommandBuffers(_vulkan_device->get_device(), &allocInfo, &commandBuffer);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create command buffer");
        return nullptr;
    }

    VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (res != VK_SUCCESS)
    {
        vkFreeCommandBuffers(_vulkan_device->get_device(), _command_pool->get_transient_command_pool(), 1, &commandBuffer);
        throw std::runtime_error("SYSTEM: Failed to create command buffer");
        return nullptr;
    }

    return commandBuffer;
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
            .x = 0, 
            .y = 0,
            .width = static_cast<f32>(width),
            .height = static_cast<f32>(height),
            .minDepth = 0,
            .maxDepth = 1
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
