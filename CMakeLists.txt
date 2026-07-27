cmake_minimum_required(VERSION 4.0)
project(get LANGUAGES CXX)

find_package(Vulkan REQUIRED COMPONENTS volk shaderc_combined)

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


set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glm glfw vma)

file(GLOB sources
    ${PROJECT_SOURCE_DIR}/src/*.h
    ${PROJECT_SOURCE_DIR}/src/*.cpp
    ${PROJECT_SOURCE_DIR}/src/vulkan/*.h
    ${PROJECT_SOURCE_DIR}/src/vulkan/*.cpp)


set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()

add_definitions(-DSOME_DEFINITIONS)
add_executable(get ${sources})

target_compile_options(get PRIVATE
    -g
    -O0
    -fsanitize=address,undefined
    -fno-omit-frame-pointer
)

target_link_options(get PRIVATE
    -fsanitize=address,undefined
)

target_link_libraries(get PRIVATE
    Vulkan::Vulkan
    Vulkan::volk
    Vulkan::shaderc_combined
    glm::glm
    glfw
    GPUOpen::VulkanMemoryAllocator
)
