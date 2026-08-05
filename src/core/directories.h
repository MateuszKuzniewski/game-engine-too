#pragma once
#include <string>
#include <filesystem>

namespace get
{
    class directories
    {
    public:

       [[nodiscard]] static std::string project_path()
       {
           return std::filesystem::current_path().string();;
       }

       [[nodiscard]] static std::string shader_path()
       {
           auto projectPath = project_path();
           return projectPath + "/shaders";
       }
    };
}
