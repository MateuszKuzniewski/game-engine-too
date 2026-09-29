#include "renderer.h"

get::renderer::renderer(
            const window& win,
            const vk_device& device,
            const vk_swapchain& swapchain,
            const vk_semaphore& semaphore,
            const vk_descriptor_set& descriptorSet,
            const depth_buffer& depthBuffer,
            VkInstance instance,
            VkPhysicalDevice physicalDevice,
            u32 queueFamilyID,
            u32 swapchainImageCount) :
        _device(&device), 
        _swapchain(&swapchain), 
        _semaphore(&semaphore),
        _descriptor_set(&descriptorSet),
        _depth_buffer(&depthBuffer)
{
    _gui =  std::make_unique<get::gui>(
                win,
                device,
                swapchain.get_format(),
                instance,
                physicalDevice,
                queueFamilyID,
                swapchainImageCount);

}

VkRenderingInfo get::renderer::prepare_frame(
        VkAttachmentLoadOp attachementLoad,
        const frame_resource& resource, 
        const frame_constants& constants,
        VkPipelineLayout pipelineLayout,
        u32 imageIndex, 
        u32 width, 
        u32 height)
{
    auto swapchain = const_cast<vk_swapchain*>(_swapchain);

    _color_attach_info = {};
    _depth_attach_info = {};

    _color_attach_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    _color_attach_info.imageView = swapchain->get_swapchain_image_views()[imageIndex];
    _color_attach_info.imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL;
    _color_attach_info.loadOp = attachementLoad,
    _color_attach_info.storeOp = VK_ATTACHMENT_STORE_OP_STORE;
    _color_attach_info.clearValue = _clear_color;

    _depth_attach_info.sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO;
    _depth_attach_info.imageView = _depth_buffer->get_image_view();
    _depth_attach_info.imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL;
    _depth_attach_info.loadOp = attachementLoad,
    _depth_attach_info.storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE;
    _depth_attach_info.clearValue  = { .depthStencil { 1.0f, 0 } };


    VkRenderingInfo renderingInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea = 
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
        .pColorAttachments = &_color_attach_info,
    };

    if (pipelineLayout != VK_NULL_HANDLE)
    {
        renderingInfo.pDepthAttachment = &_depth_attach_info;
        vkCmdPushConstants(
                resource.command_buffer,
                pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
                0, 
                sizeof(get::frame_constants), 
                &constants);

        auto gds = _descriptor_set->get_global_descriptor_set();  
        vkCmdBindDescriptorSets(
                resource.command_buffer,
                VK_PIPELINE_BIND_POINT_GRAPHICS,
                pipelineLayout,
                0,
                1,
                &gds,
                0,
                nullptr);
    }

    return renderingInfo;
}

void get::renderer::submit(const frame_resource& resource, VkPipeline pipeline, VkRenderingInfo renderingInfo, u32 width, u32 height, u32 drawIndex)
{
    vkCmdBeginRendering(resource.command_buffer, &renderingInfo);
    {
        VkViewport viewport
        {
            .x = 0, 
            .y = static_cast<f32>(height),
            .width = static_cast<f32>(width),
            .height = -static_cast<f32>(height),
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

        vkCmdSetScissor(resource.command_buffer, 0, 1, &scissor);
        vkCmdBindPipeline(resource.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, pipeline);
        vkCmdDrawIndexedIndirect(resource.command_buffer, resource.indirect_draw_buffer.buffer, 0, drawIndex, sizeof(VkDrawIndexedIndirectCommand));

    }
    vkCmdEndRendering(resource.command_buffer);
}

void get::renderer::submit(const frame_resource& resource, VkRenderingInfo renderingInfo, const render_debug_info& debugInfo)
{
    vkCmdBeginRendering(resource.command_buffer, &renderingInfo);
    {
        _gui->render(resource.command_buffer, debugInfo);
    }
    vkCmdEndRendering(resource.command_buffer);
}

void get::renderer::present(const frame_resource& resource, u32 imageIndex, VkSemaphore imageAcquireSemaphore, u32 signalValue)
{
    auto swapchain = const_cast<vk_swapchain*>(_swapchain);
    auto semaphore = const_cast<vk_semaphore*>(_semaphore);

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
            .image = swapchain->get_swapchain_images()[imageIndex],
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
            .semaphore = swapchain->get_render_complete_semaphores()[imageIndex],
            .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
        },
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = semaphore->get_semaphore(),
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

    vkQueueSubmit2(_device->get_queue(), 1, &submitInfo, VK_NULL_HANDLE);

    const auto swapchainKHR = swapchain->get_swapchain();
    VkPresentInfoKHR presentInfo
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &swapchain->get_render_complete_semaphores()[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &swapchainKHR,
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };

    vkQueuePresentKHR(_device->get_queue(), &presentInfo);
}
