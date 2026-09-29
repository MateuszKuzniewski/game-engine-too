#pragma once
#include <volk.h>
#include <memory>
#include "vk_device.h"
#include "vk_swapchain.h"
#include "vk_semaphore.h"
#include "vk_descriptor_set.h"
#include "depth_buffer.h"
#include "render_data.h"
#include "gui.h"

namespace get
{
    class renderer
    {
    public:

        renderer(
            const window& win,
            const vk_device& device,
            const vk_swapchain& swapchain,
            const vk_semaphore& semaphore,
            const vk_descriptor_set& descriptorSet,
            const depth_buffer& depthBuffer,
            VkInstance instance,
            VkPhysicalDevice physicalDevice,
            u32 queueFamilyID,
            u32 swapchainImageCount);

        ~renderer() = default;

        renderer(const renderer&) = delete;
        renderer(renderer&&) = delete;
        renderer& operator=(const renderer&) = delete;
        renderer& operator=(renderer&&) = delete;

        [[nodiscard]] VkRenderingInfo prepare_frame(
                VkAttachmentLoadOp attachementLoad,
                const frame_resource& resource, 
                const frame_constants& constants,
                VkPipelineLayout pipelineLayout,
                u32 imageIndex, 
                u32 width, 
                u32 height);

        void submit(const frame_resource& resource, VkPipeline pipeline, VkRenderingInfo renderingInfo, u32 width, u32 height, u32 drawIndex);
        void submit(const frame_resource& resource, VkRenderingInfo renderingInfo, const render_debug_info& debugInfo);

        void present(const frame_resource& resource, u32 imageIndex, VkSemaphore imageAcquireSemaphore, u32 signalValue);

    private:

        VkClearValue _clear_color
        {
            .color = {{ 0.02f, 0.9f, 0.94f, 1.0f }},
        };

        // TO DO: This won't work with multithreaded renderer but it's fine for now
        VkRenderingAttachmentInfo _color_attach_info {};
        VkRenderingAttachmentInfo _depth_attach_info {};

        std::unique_ptr<gui> _gui;

        const vk_device* _device;
        const vk_swapchain* _swapchain;
        const vk_semaphore* _semaphore;
        const vk_descriptor_set* _descriptor_set;
        const depth_buffer* _depth_buffer;
    };
}
