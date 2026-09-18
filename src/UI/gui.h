#pragma once
#include <volk.h>
#include <imgui.h>
#include "window.h"
#include "render_data.h"

namespace get
{
    class gui
    {
    public:

        gui(const window& win,
            VkFormat format,
            VkInstance instance,
            VkPhysicalDevice physicalDevice,
            VkDevice device,
            u32 queueFamilyID,
            VkQueue queue,
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
