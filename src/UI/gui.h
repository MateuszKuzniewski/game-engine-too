#pragma once
#include <volk.h>
#include <imgui.h>
#include <vk_device.h>
#include "window.h"
#include "render_data.h"

namespace get
{
    class gui
    {
    public:

        gui(const window& win,
            const vk_device& device,
            VkFormat format,
            VkInstance instance,
            VkPhysicalDevice physicalDevice,
            u32 queueFamilyID,
            u32 swapchainImageCount);

        ~gui();
        
        gui(const gui&) = delete;
        gui(gui&&) = delete;
        gui& operator=(const gui&) = delete;
        gui& operator=(gui&&) = delete;
    
        void render(VkCommandBuffer commandBuffer, const render_debug_info& info);

    
    private:

        void setup();
        void prepare_debug_panel(const render_debug_info& info) const;

        static void check_vk_result(VkResult err);

    private:

        VkDevice _device;
        VkDescriptorPool _imgui_pool;
    };
}
