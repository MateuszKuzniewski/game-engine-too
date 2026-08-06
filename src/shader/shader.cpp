#include <stdexcept>
#include <fstream>
#include <sstream>
#include "types.h"
#include "shader.h"
#include "directories.h"


get::shader::shader(VkDevice device, const std::string& vertshader, const std::string& fragShader)
    :   _vert_shader_name(vertshader),
        _frag_shader_name(fragShader),
        _device(device)
{
}

VkShaderModule get::shader::compile(shader_type type) const
{
    std::string filename = "";
    switch (type)
    {
        case shader_type::VERT: filename = _vert_shader_name; break;
        case shader_type::FRAG: filename = _frag_shader_name; break;
    }

    auto shaderPath = std::filesystem::path(get::directories::shader_path()) / filename;
    // std::string shaderPath = "/home/v/dev/git/game-engine-too/shaders/" + filename;
    shaderPath = std::filesystem::path(get::directories::shader_path()) / filename;
    const std::string src = read_file(shaderPath);
    
    auto kind = convert_shader_type(type);

    if (src.empty())
    {
        throw std::runtime_error("SYSTEM: Failed to read shader file: " + filename);
    }

    shaderc::Compiler compiler;
    shaderc::CompileOptions options;

    options.SetTargetEnvironment(shaderc_target_env_vulkan, shaderc_env_version_vulkan_1_4);
    options.SetTargetSpirv(shaderc_spirv_version_1_6);
    options.SetOptimizationLevel(shaderc_optimization_level_performance);
    shaderc::CompilationResult res = compiler.CompileGlslToSpv(src, kind, filename.c_str(), options);

    if (res.GetCompilationStatus() != shaderc_compilation_status_success)
    {
        throw std::runtime_error("SYSTEM: Shader compalition error: " + res.GetErrorMessage());
    }

    const size_t shaderSize = (res.cend() - res.cbegin()) * sizeof(u32);
    VkShaderModuleCreateInfo moduleCreateInfo
    {
        .sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO,
        .codeSize = shaderSize,
        .pCode = res.cbegin()
    };
    
    VkShaderModule module = VK_NULL_HANDLE;
    VkResult result = vkCreateShaderModule(_device, &moduleCreateInfo, nullptr, &module);
    if (result != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create shader module");
    }

    return module;
}

shaderc_shader_kind get::shader::convert_shader_type(shader_type type) const
{
    switch (type)
    {
        case shader_type::VERT: return shaderc_vertex_shader;
        case shader_type::FRAG: return shaderc_fragment_shader;
    }
}

std::string get::shader::read_file(const std::string& path) const
{
    std::ifstream file(path);
    if (file.is_open())
    {
        std::stringstream buffer;
        buffer << file.rdbuf();
        const std::string output = buffer.str();
        file.close();
        return output;
    }

    return std::string();
}
