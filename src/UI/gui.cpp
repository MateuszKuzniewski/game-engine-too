#include "gui.h"
#include <imgui_impl_vulkan.h>
#include <imgui_impl_glfw.h>
#include <iostream>

get::gui::gui(
        const window& win,
        VkFormat format,
        VkInstance instance,
        VkPhysicalDevice physicalDevice,
        VkDevice device,
        u32 queueFamilyID,
        VkQueue queue,
        u32 swapchainImageCount) : _device(device)
{

    // From ImGui example code
    VkDescriptorPoolSize pool_sizes[] =
    {
        { VK_DESCRIPTOR_TYPE_SAMPLED_IMAGE, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLED_IMAGE_POOL_SIZE },
        { VK_DESCRIPTOR_TYPE_SAMPLER, IMGUI_IMPL_VULKAN_MINIMUM_SAMPLER_POOL_SIZE },
    };

    VkDescriptorPoolCreateInfo pool_info = {};
    pool_info.sType = VK_STRUCTURE_TYPE_DESCRIPTOR_POOL_CREATE_INFO;
    pool_info.flags = VK_DESCRIPTOR_POOL_CREATE_FREE_DESCRIPTOR_SET_BIT;
    pool_info.maxSets = 0;
    for (VkDescriptorPoolSize& pool_size : pool_sizes)
        pool_info.maxSets += pool_size.descriptorCount;
    pool_info.poolSizeCount = (uint32_t)IM_COUNTOF(pool_sizes);
    pool_info.pPoolSizes = pool_sizes;

    VkResult err = vkCreateDescriptorPool(_device, &pool_info, nullptr, &_imgui_pool);
    check_vk_result(err);
            
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGui::StyleColorsDark();
    ImGui_ImplGlfw_InitForVulkan(win.get_current_window(), true);
        
    VkPipelineRenderingCreateInfoKHR pipelineRenderInfo
    {
        .sType = VK_STRUCTURE_TYPE_PIPELINE_RENDERING_CREATE_INFO_KHR,
        .colorAttachmentCount = 1,
        .pColorAttachmentFormats = &format
    };

    ImGui_ImplVulkan_PipelineInfo pipelineInfo
    {
        .Subpass = 0,
        .MSAASamples = VK_SAMPLE_COUNT_1_BIT,
        .PipelineRenderingCreateInfo = pipelineRenderInfo,
    };

    ImGui_ImplVulkan_InitInfo initInfo
    {
        .Instance = instance,
        .PhysicalDevice = physicalDevice,
        .Device = device,
        .QueueFamily = queueFamilyID,
        .Queue = queue,
        .DescriptorPool = _imgui_pool,
        .MinImageCount = swapchainImageCount,
        .ImageCount = swapchainImageCount,
        .PipelineInfoMain = pipelineInfo,
        .UseDynamicRendering = true,
        .CheckVkResultFn = check_vk_result
    };
    
    ImGui_ImplVulkan_Init(&initInfo);
}

get::gui::~gui()
{
    ImGui_ImplVulkan_Shutdown();
    ImGui_ImplGlfw_Shutdown();
    ImGui::DestroyContext();
    vkDestroyDescriptorPool(_device, _imgui_pool, nullptr);
}

void get::gui::render(VkCommandBuffer commandBuffer, const render_debug_info& info)
{
    setup();
    
    prepare_debug_panel(info);

    ImGui::Render();

    ImGui_ImplVulkan_RenderDrawData(ImGui::GetDrawData(), commandBuffer);
}

void get::gui::setup()
{
    ImGui_ImplVulkan_NewFrame();
    ImGui_ImplGlfw_NewFrame();
    ImGui::NewFrame();
}

void get::gui::prepare_debug_panel(const render_debug_info& info) const
{
    ImGui::Begin("Debug");
    ImGui::Text("Average Time: %.3f ms", 1000.0f / ImGui::GetIO().Framerate);
    ImGui::Text("FPS: %.1f", ImGui::GetIO().Framerate);
    ImGui::Text("SubMesh Count: %lu", info.sub_mesh_count);
    ImGui::End();
}

void get::gui::check_vk_result(VkResult err) 
{
    if (err == VK_SUCCESS) 
        return;
        
    std::cerr << "SYSTEM: ImGui ERROR VkResult = " << err << std::endl;
    if (err < 0) 
    {
        abort(); 
    }
}
