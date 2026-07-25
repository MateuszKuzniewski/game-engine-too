cmake_minimum_required(VERSION 4.0)
project(game-engine-too LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 23)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# Always emit compile_commands.json for clangd (neovim LSP)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)

# Default to a debug build if nothing was specified
if(NOT CMAKE_BUILD_TYPE AND NOT CMAKE_CONFIGURATION_TYPES)
    set(CMAKE_BUILD_TYPE Debug CACHE STRING "Build type" FORCE)
endif()


find_package(Vulkan REQUIRED)

include(FetchContent)

# GLM - header-only math library
FetchContent_Declare(
    glm
    GIT_REPOSITORY https://github.com/g-truc/glm.git
    GIT_TAG        1.0.1
)

# vk-bootstrap - cuts Vulkan instance/device/swapchain boilerplate
FetchContent_Declare(
    vk-bootstrap
    GIT_REPOSITORY https://github.com/charles-lunarg/vk-bootstrap.git
    GIT_TAG        v1.3.302
)

# GLFW - windowing/input
FetchContent_Declare(
    glfw
    GIT_REPOSITORY https://github.com/glfw/glfw.git
    GIT_TAG        3.4
)
set(GLFW_BUILD_EXAMPLES OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_TESTS OFF CACHE BOOL "" FORCE)
set(GLFW_BUILD_DOCS OFF CACHE BOOL "" FORCE)

FetchContent_MakeAvailable(glm vk-bootstrap glfw)

# ---------------------------------------------------------------------------
# Source files
# ---------------------------------------------------------------------------

file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_SOURCE_DIR}/src/*.hpp"
    "${CMAKE_SOURCE_DIR}/src/*.h"
)

add_executable(${PROJECT_NAME} ${ENGINE_SOURCES})

target_include_directories(${PROJECT_NAME} PRIVATE
    "${CMAKE_SOURCE_DIR}/src"
)

target_link_libraries(${PROJECT_NAME} PRIVATE
    Vulkan::Vulkan
    glm::glm
    vk-bootstrap::vk-bootstrap
    glfw
)

target_compile_definitions(${PROJECT_NAME} PRIVATE
    GLFW_INCLUDE_VULKAN
    GLM_FORCE_RADIANS
    GLM_FORCE_DEPTH_ZERO_TO_ONE
)

if(MSVC)
    target_compile_options(${PROJECT_NAME} PRIVATE /W4)
else()
    target_compile_options(${PROJECT_NAME} PRIVATE -Wall -Wextra -Wpedantic)
endif()

# ---------------------------------------------------------------------------
# Shader compilation (GLSL -> SPIR-V via glslangValidator/glslc from the SDK)
# ---------------------------------------------------------------------------

find_program(GLSL_VALIDATOR glslangValidator HINTS
    ${Vulkan_INCLUDE_DIRS}/../Bin
    $ENV{VULKAN_SDK}/Bin
    $ENV{VULKAN_SDK}/bin
)

file(GLOB_RECURSE SHADER_SOURCES CONFIGURE_DEPENDS
    "${CMAKE_SOURCE_DIR}/shaders/*.vert"
    "${CMAKE_SOURCE_DIR}/shaders/*.frag"
    "${CMAKE_SOURCE_DIR}/shaders/*.comp"
)

set(SPIRV_BINARY_FILES "")
foreach(SHADER ${SHADER_SOURCES})
    get_filename_component(SHADER_NAME ${SHADER} NAME)
    set(SPIRV "${CMAKE_BINARY_DIR}/shaders/${SHADER_NAME}.spv")
    add_custom_command(
        OUTPUT ${SPIRV}
        COMMAND ${CMAKE_COMMAND} -E make_directory "${CMAKE_BINARY_DIR}/shaders"
        COMMAND ${GLSL_VALIDATOR} -V ${SHADER} -o ${SPIRV}
        DEPENDS ${SHADER}
        VERBATIM
    )
    list(APPEND SPIRV_BINARY_FILES ${SPIRV})
endforeach()

if(SPIRV_BINARY_FILES)
    add_custom_target(shaders DEPENDS ${SPIRV_BINARY_FILES})
    add_dependencies(${PROJECT_NAME} shaders)
endif()
