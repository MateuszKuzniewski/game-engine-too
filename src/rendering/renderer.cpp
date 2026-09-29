#include "renderer.h"

get::renderer::renderer(const window& win,
                VkFormat format,
                VkInstance instance,
                VkPhysicalDevice physicalDevice,
                VkDevice device,
                u32 queueFamilyID,
                VkQueue queue,
                u32 swapchainImageCount)
{
    _gui =  std::make_unique<get::gui>(
            win,
            format,
            instance,
            physicalDevice,
            device,
            queueFamilyID,
            queue,
            swapchainImageCount);

}

get::renderer::~renderer()
{

}


void get::renderer::submit(VkPipeline pipeline, VkRenderingInfo renderingInfo, const frame_resource& resource, u32 width, u32 height, u32 drawIndex)
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


void get::renderer::renderer::submit(const frame_resource& resource, VkRenderingInfo renderingInfo, const render_debug_info& debugInfo)
{
    vkCmdBeginRendering(resource.command_buffer, &renderingInfo);
    {
        _gui->render(resource.command_buffer, debugInfo);
    }
    vkCmdEndRendering(resource.command_buffer);

}
