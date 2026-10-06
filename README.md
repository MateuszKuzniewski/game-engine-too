# Game Engine Too (GET)
A modern Vulkan rendering engine written in C++, built around a GPU-driven rendering pipeline.

![alt text](https://github.com/MateuszKuzniewski/game-engine-too/blob/main/preview.png)

## Features
- Dynamic Rendering
- Bindless Textures
- Indirect Drawing
- Simple shading
- glTF 2.0 support

## Requirements
- VulkanSDK 1.3 or newer
- C++ 23 or newer
- Shaderc (Comes with SDK on windows, install seperately on Linux)
- Clangd 22.0 or newer
- CMake 4.0 or newer
- Ninja

## Dependencies
- Vulkan SDK
- GLFW
- GLM
- tinyglTF
- VMA
- stbimage
- ImGui

## How to build
```shell
cmake -S . -B build -G "Ninja Multi-Config" 
```
Change "Debug" to "Release" for release build

```shell
cmake --build build --config Debug
```
### Add assets folder
To render a scene, add following folder structure to the root folder
```shell
assets/models/your-folder
```
then go to application.cpp and change the line 223
```shell
const std::string your_model = "models/your_folder/your_scene.gltf";

std::filesystem::path path = get::directories::asset_path() / your_model;
```

This is a crude solution which will change in the future

## Note on Linux builds
If you run the debug build on Linux, LSAN might pick up leaks from 3rd party libraries that GLFW uses, run it with 
```shell
LSAN_OPTIONS=suppressions=lsan_suppressions.txt ./build/Debug/get
```

or run the bash scripts

## Controls
- W S A D -> Camera move
- Arrows -> Camera rotate
- Space -> Up
- c -> Down