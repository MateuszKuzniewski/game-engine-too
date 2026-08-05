#pragma once
#include <volk.h>
#include <shaderc/shaderc.hpp>
#include <string>

namespace get 
{
    enum class shader_type
    {
        VERT = 0,
        FRAG
    };

    class shader
    {
    public:

        shader(VkDevice device, const std::string& vertshader, const std::string& fragShader);
        ~shader() = default;
    
        shader(const shader&) = delete;
        shader(shader&&) = delete;
        shader& operator=(const shader&) = delete;
        shader& operator=(shader&&) = delete;

        void compile(const std::string& filename, const shader_type type, VkShaderModule module);

        [[nodiscard]] VkShaderModule get_shader_module() const;

    private:

        [[nodiscard]] std::string read_file(const std::string& path) const;
        shaderc_shader_kind convert_shader_type(shader_type type);

    private:

        VkDevice _device;
        VkShaderModule _vert_shader_module;
        VkShaderModule _frag_shader_module;
    };
}
