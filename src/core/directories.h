#pragma once
#include <filesystem> 
#include <stdexcept>

#if defined(_WIN32)
  #include <windows.h>
#elif defined(__APPLE__)
  #include <mach-o/dyld.h>
  #include <vector>
#elif defined(__linux__)
  #include <unistd.h>
  #include <limits.h>
#endif


namespace get
{
    class directories
    {
    public:

       [[nodiscard]] static std::filesystem::path project_path()
       {
#if defined(_WIN32)
            wchar_t buf[MAX_PATH];
            DWORD len = GetModuleFileNameW(nullptr, buf, MAX_PATH);
            if (len == 0 || len == MAX_PATH)
                throw std::runtime_error("Failed to get executable path");
            return std::filesystem::path(buf, buf + len);

#elif defined(__APPLE__)
            uint32_t size = 0;
            _NSGetExecutablePath(nullptr, &size); // first call just gets required size
            std::vector<char> buf(size);
            if (_NSGetExecutablePath(buf.data(), &size) != 0)
                throw std::runtime_error("Failed to get executable path");
            return std::filesystem::canonical(buf.data());

#elif defined(__linux__)
            char buf[PATH_MAX];
            ssize_t len = readlink("/proc/self/exe", buf, sizeof(buf) - 1);
            if (len == -1)
                throw std::runtime_error("Failed to get executable path");
            buf[len] = '\0';
            return std::filesystem::path(buf).parent_path().parent_path();

#else
            #error "Unsupported platform"
#endif       
       }

       [[nodiscard]] static std::filesystem::path shader_path()
       {
           auto projectPath = project_path();
           return projectPath / "shaders/";
       }

       [[nodiscard]] static std::filesystem::path asset_path()
       {
           auto projectPath = project_path();
           return projectPath / "assets/"; 
       }
    };
}
