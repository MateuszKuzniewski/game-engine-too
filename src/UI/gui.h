#pragma once
#include <volk.h>
#include <imgui.h>
#include "window.h"

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
    
        void render(VkCommandBuffer commandBuffer);

    
    private:

        static void check_vk_result(VkResult err);
        void setup();

    private:

        VkDevice _device;
        VkDescriptorPool _imgui_pool;
    };
}
