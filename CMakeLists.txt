cmake_minimum_required(VERSION 4.0)
project(get LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# --- Build type setup ---------------------------------------------------
# Supported configs: Debug (sanitizers on non-Windows, no optimization),
# Release (optimized, no sanitizers), RelWithDebInfo (optimized + debug
# symbols, no sanitizers).
set(CMAKE_CONFIGURATION_TYPES "Debug;Release;RelWithDebInfo" CACHE STRING "" FORCE)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Release CACHE STRING "Build type" FORCE)
    set_property(CACHE CMAKE_BUILD_TYPE PROPERTY STRINGS "Debug" "Release" "RelWithDebInfo")
endif()

# --- Vulkan / volk / shaderc ----------------------------------------------
# We always link shaderc_shared (on both platforms). Unlike shaderc_combined,
# the shared library is self-contained, so distro packages (e.g. Arch) don't
# produce undefined glslang / SPIRV-Tools symbols at link time.
if (WIN32)
    find_package(Vulkan REQUIRED)
else()
    find_package(Vulkan REQUIRED COMPONENTS volk)
endif()

find_library(SHADERC_SHARED_LIB
    NAMES shaderc_shared
    HINTS "$ENV{VULKAN_SDK}/Lib" "$ENV{VULKAN_SDK}/lib"
    REQUIRED
)

include(FetchContent)

FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG 1.0.1
)

FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG 3.4
)

FetchContent_Declare(
    vma
    GIT_REPOSITORY https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator
    GIT_TAG v3.4.0
)

FetchContent_Declare(
    gltf
    GIT_REPOSITORY https://github.com/syoyo/tinygltf
    GIT_TAG v3.0.1
)

set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glm glfw vma gltf)

# CONFIGURE_DEPENDS makes the build re-glob when files are added/removed.
file(GLOB sources CONFIGURE_DEPENDS
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/ext/*.h
    ${PROJECT_SOURCE_DIR}/ext/*.cpp
    ${PROJECT_SOURCE_DIR}/ext/imgui/*.h
    ${PROJECT_SOURCE_DIR}/ext/imgui/*.cpp
    ${PROJECT_SOURCE_DIR}/ext/imgui/backends/*.h
    ${PROJECT_SOURCE_DIR}/ext/imgui/backends/*.cpp
    ${PROJECT_SOURCE_DIR}/src/vulkan/*.h
    ${PROJECT_SOURCE_DIR}/src/vulkan/*.cpp
    ${PROJECT_SOURCE_DIR}/src/glfw/*.h
    ${PROJECT_SOURCE_DIR}/src/glfw/*.cpp
    ${PROJECT_SOURCE_DIR}/src/buffer/*.h
    ${PROJECT_SOURCE_DIR}/src/buffer/*.cpp
    ${PROJECT_SOURCE_DIR}/src/shader/*.h
    ${PROJECT_SOURCE_DIR}/src/shader/*.cpp
    ${PROJECT_SOURCE_DIR}/src/core/*.h
    ${PROJECT_SOURCE_DIR}/src/core/*.cpp
    ${PROJECT_SOURCE_DIR}/src/gltf/*.h
    ${PROJECT_SOURCE_DIR}/src/gltf/*.cpp
    ${PROJECT_SOURCE_DIR}/src/rendering/*.h
    ${PROJECT_SOURCE_DIR}/src/rendering/*.cpp
    ${PROJECT_SOURCE_DIR}/src/input/*.h
    ${PROJECT_SOURCE_DIR}/src/input/*.cpp
    ${PROJECT_SOURCE_DIR}/src/UI/*.h
    ${PROJECT_SOURCE_DIR}/src/UI/*.cpp
)

add_executable(get ${sources})

target_include_directories(get PRIVATE
    ${PROJECT_SOURCE_DIR}/src
    ${PROJECT_SOURCE_DIR}/ext
    ${PROJECT_SOURCE_DIR}/ext/imgui
    ${PROJECT_SOURCE_DIR}/ext/imgui/backends
    ${PROJECT_SOURCE_DIR}/src/buffer
    ${PROJECT_SOURCE_DIR}/src/glfw
    ${PROJECT_SOURCE_DIR}/src/shader
    ${PROJECT_SOURCE_DIR}/src/vulkan
    ${PROJECT_SOURCE_DIR}/src/core
    ${PROJECT_SOURCE_DIR}/src/gltf
    ${PROJECT_SOURCE_DIR}/src/rendering
    ${PROJECT_SOURCE_DIR}/src/input
    ${PROJECT_SOURCE_DIR}/src/UI
)

# Windows only: the SDK's volk header lives in a subdirectory, and there's no
# Vulkan::volk target from the plain find_package(Vulkan) call above.
if (WIN32)
    target_include_directories(get PRIVATE "${Vulkan_INCLUDE_DIR}/volk" "${Vulkan_INCLUDE_DIR}")
endif()

# --- Per-config compile/link options -------------------------------------
# Debug: no optimization, debug symbols, sanitizers (non-Windows only).
# RelWithDebInfo: optimized, debug symbols kept, no sanitizers.
# Release: fully optimized, no debug info, no sanitizers, NDEBUG defined
# (strips asserts). Use this for actual FPS numbers / benchmarking.
target_compile_options(get PRIVATE
    $<$<CONFIG:Debug>:-g -O0>
    $<$<CONFIG:RelWithDebInfo>:-g -O3>
    $<$<CONFIG:Release>:-O3>
)

if (NOT WIN32)
    target_compile_options(get PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined -fno-omit-frame-pointer>
    )
    target_link_options(get PRIVATE
        $<$<CONFIG:Debug>:-fsanitize=address,undefined>
    )
endif()

target_compile_definitions(get PRIVATE
    VK_NO_PROTOTYPES
    IMGUI_IMPL_VULKAN_USE_VOLK
    $<$<CONFIG:Release>:NDEBUG>
    $<$<CONFIG:RelWithDebInfo>:NDEBUG>
)

# --- Linking -------------------------------------------------------------
if (WIN32)
    target_link_libraries(get PRIVATE
        ${SHADERC_SHARED_LIB}
        glm::glm
        glfw
        GPUOpen::VulkanMemoryAllocator
        stdc++exp
    )

    # shaderc_shared.dll must sit next to the executable at runtime.
    add_custom_command(TARGET get POST_BUILD
        COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "$ENV{VULKAN_SDK}/Bin/shaderc_shared.dll"
        $<TARGET_FILE_DIR:get>
        COMMENT "Copying shaderc_shared.dll next to executable output target..."
    )
else()
    target_link_libraries(get PRIVATE
        Vulkan::Vulkan
        Vulkan::volk
        ${SHADERC_SHARED_LIB}
        glm::glm
        glfw
        GPUOpen::VulkanMemoryAllocator
    )
endif()
