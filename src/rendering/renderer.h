#pragma once
#include <volk.h>
#include <memory>
#include "vk_device.h"
#include "vk_swapchain.h"
#include "vk_semaphore.h"
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
        VkInstance instance,
        VkPhysicalDevice physicalDevice,
        u32 queueFamilyID,
        u32 swapchainImageCount);

        ~renderer() = default;

        renderer(const renderer&) = delete;
        renderer(renderer&&) = delete;
        renderer& operator=(const renderer&) = delete;
        renderer& operator=(renderer&&) = delete;


        void submit(const frame_resource& resource, VkPipeline pipeline, VkRenderingInfo renderingInfo, u32 width, u32 height, u32 drawIndex);
        void submit(const frame_resource& resource, VkRenderingInfo renderingInfo, const render_debug_info& debugInfo);

        void present(const frame_resource& resource, u32 imageIndex, VkSemaphore imageAcquireSemaphore, u32 signalValue);

    private:

        std::unique_ptr<gui> _gui;
        const vk_device* _device;
        const vk_swapchain* _swapchain;
        const vk_semaphore* _semaphore;
    };
}
