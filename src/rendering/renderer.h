#pragma once
#include <volk.h>
#include <memory>
#include "render_data.h"
#include "gui.h"

namespace get
{
    class renderer
    {
    public:

        renderer(const window& win,
                VkFormat format,
                VkInstance instance,
                VkPhysicalDevice physicalDevice,
                VkDevice device,
                u32 queueFamilyID,
                VkQueue queue,
                u32 swapchainImageCount);

        ~renderer();

        void submit(VkPipeline pipeline, VkRenderingInfo renderingInfo, const frame_resource& resource, u32 width, u32 height, u32 drawIndex);
        void submit(const frame_resource& resource, VkRenderingInfo renderingInfo, const render_debug_info& debugInfo);

    private:
        std::unique_ptr<gui> _gui;
    };
}
