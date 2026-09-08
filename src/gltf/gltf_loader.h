#pragma once
#include <string>

namespace get
{
    class gltf_loader
    {
    public:

        gltf_loader(const std::string& filepath);
        ~gltf_loader();
    };
}
