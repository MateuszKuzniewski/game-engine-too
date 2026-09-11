#include <print>
#include <unordered_map>
#include <assert.h>
#include "application.h"
#include "directories.h"
#include "frame_time.h"
#include "stb_image.h"
#include "glm/gtc/type_ptr.hpp"


application::application() : _frame_index(0), _max_frames_in_flight(2), _next_signal_value(2), _frame_resources(2)
{
    std::println("{0}", "SYSTEM: Application was created");
    std::println("{0}{1}", "SYSTEM: Project path is set to: ", get::directories::project_path());
    std::println("{0}{1}", "SYSTEM: Shader path is set to: ", get::directories::shader_path());

    get::window_settings settings
    {
        .width = 1280,
        .height = 720, 
        .title = "Game Engine Too"
    };

    get::camera_settings cameraSettings
    {
        .fov = 60.f,
        .near_clip = 0.1f,
        .far_clip = 10000.f,
    };
    
    _glfw_context =     std::make_unique<get::glfw_context>();
    _vulkan_context =   std::make_unique<get::vk_context>(*_glfw_context, settings.title);
    _window =           std::make_unique<get::window>(settings);
    _surface =          std::make_unique<get::vk_surface>(*_window, _vulkan_context->get_instance());
    _physical_device =  std::make_unique<get::vk_physical_device>(_vulkan_context->get_instance());
    _queue_family =     std::make_unique<get::vk_queue_family>(_physical_device->get_device(), _surface->get_surface());
    _vulkan_device =    std::make_unique<get::vk_device>(_physical_device->get_device(), _queue_family->get_queue_family_id());
    _vma =              std::make_unique<get::vk_memory_allocator>(
                            _vulkan_context->get_instance(), 
                            _physical_device->get_device(), 
                            _vulkan_device->get_device());

    _swapchain =        std::make_unique<get::vk_swapchain>(
                            _vulkan_device->get_device(), 
                            _physical_device->get_device(),
                            _surface->get_surface());

    _swapchain->create(settings.width, settings.height);

    _depth_buffer =     std::make_unique<get::depth_buffer>(
                            _vulkan_device->get_device(), 
                            _vma->get_allocator(), 
                            settings.width, 
                            settings.height);

    _depth_buffer->create(settings.width, settings.height);

    _shader =           std::make_unique<get::shader>(
                            _vulkan_device->get_device(),
                            "shader.vert",
                            "shader.frag");

    _descriptor_set =   std::make_unique<get::vk_descriptor_set>(_vulkan_device->get_device(), MAX_TEXTURES);

    _vulkan_pipeline =  std::make_unique<get::vk_pipeline>( _vulkan_device->get_device(), *_shader, _descriptor_set->get_descriptor_set_layout());

    _semaphore =        std::make_unique<get::vk_sempahore>(_vulkan_device->get_device(), _frame_resources, _max_frames_in_flight);

    _command_pool =     std::make_unique<get::command_pool>(_vulkan_device->get_device(), _queue_family->get_queue_family_id(), _frame_resources);

    _command_buffer =   std::make_unique<get::command_buffer>(_vulkan_device->get_device(), _frame_resources);

    _main_camera =      std::make_unique<get::camera>(settings.width, settings.height, cameraSettings);
    
    _node_world =       std::make_unique<get::node_world>(1024);
    
    create_indirect_buffers();
}

application::~application()
{
    vkDeviceWaitIdle(_vulkan_device->get_device());
    
    auto device = _vulkan_device->get_device();
    auto allocator = _vma->get_allocator();

    for (auto buffer : _buffers)
    {
        vkDestroyBuffer(device, buffer.buffer, nullptr);
        vmaFreeMemory(allocator, buffer.allocation);
    }

    for (auto sampler : _samplers)
    {
        vkDestroySampler(device, sampler, nullptr);
    }

    for (auto& img : _gpu_images)
    {
        vkDestroyImageView(device, img.image_view, nullptr);
        vkDestroyImage(device, img.image, nullptr);
        vmaFreeMemory(allocator, img.allocation);
    }

    for (auto& res : _frame_resources)
    {
        vkDestroySemaphore(device, res.image_acquired_semaphore, nullptr);
        vkDestroyCommandPool(device, res.command_pool, nullptr);

        vmaUnmapMemory(allocator, res.indirect_draw_buffer.allocation);
        vkDestroyBuffer(device, res.indirect_draw_buffer.buffer, nullptr);
        vmaFreeMemory(allocator, res.indirect_draw_buffer.allocation);

        vmaUnmapMemory(allocator, res.render_item_buffer.allocation);
        vkDestroyBuffer(device, res.render_item_buffer.buffer, nullptr);
        vmaFreeMemory(allocator, res.render_item_buffer.allocation);
    }

    _gpu_images.clear();
    _vertices.clear();
    _indices.clear();

    std::println("{0}", "SYSTEM: Application was destroyed");
}

void application::load_data()
{
    constexpr size_t vertSizeMB = 128;
    constexpr size_t indicesSizeMB = 64;
    constexpr size_t vertexBufferBytes = static_cast<size_t>(vertSizeMB * 1024 * 1024); // vertex buffer size
    constexpr size_t indexBufferBytes = static_cast<size_t>(indicesSizeMB * 1024 * 1024); // index buffer size
    constexpr size_t totalVerts = vertexBufferBytes / sizeof(get::vertex);
    constexpr size_t totalIndices = indexBufferBytes / sizeof(u32);

    _vertices.resize(totalVerts);
    _indices.resize(totalIndices);

    u32 whitePixelData = 0xFFFFFFFF; // RGBA
    get::image fallbackTextureColor
    {
        .width = 1,
        .height = 1,
        .channels = 4,
        .data = reinterpret_cast<unsigned char*>(&whitePixelData)
    };

    VkCommandBuffer imageBuffer = start_transient_command_buffer();
    auto [whiteImageId, whiteStagingBuffer] = create_image(imageBuffer, fallbackTextureColor.data, fallbackTextureColor.width, fallbackTextureColor.height, 4);

    _fallback_image_id = whiteImageId;

    submit_transient_command_buffer(imageBuffer);

    _vma->destroy(whiteStagingBuffer.buffer, whiteStagingBuffer.allocation);

    VkSamplerCreateInfo samplerInfo
    {
        .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
        .magFilter = VK_FILTER_NEAREST,
        .minFilter = VK_FILTER_NEAREST,
        .addressModeU = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeV = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
        .compareEnable = VK_FALSE
    };

    VkSampler sampler = nullptr;

    VkResult res = vkCreateSampler(_vulkan_device->get_device(), &samplerInfo, nullptr, &sampler);

    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create sampler");
    }

    _samplers.push_back(sampler);
    u32 fallbackSamplerID = _samplers.size();
    _textures.push_back(get::texture { .image_id = whiteImageId, .sampler_id = fallbackSamplerID });

    load_gltf(get::directories::asset_path() + "/models/car/scene.gltf");
    // load_gltf(get::directories::asset_path() + "/models/mario/scene.gltf");
    
    get::node& root = _node_world->get_node(_root_node_id);
    root.set_scale(glm::vec3(0.1, 0.1, 0.1));
    root.set_translation(glm::vec3(0, -10, -100));

    auto x = glm::rotate(root.get_rotation(), glm::radians(45.0f), glm::vec3(0,0,1));
    root.set_rotation(x);

    get::gpu_buffer vertexBufferStage = create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, vertexBufferBytes, true, VMA_MEMORY_USAGE_AUTO);
    if (!vertexBufferStage.buffer)
    {
        throw std::runtime_error("SYSTEM: Failed to create vertex staging buffer");
    }

    get::gpu_buffer indexBufferStage = create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, indexBufferBytes, true, VMA_MEMORY_USAGE_AUTO);
    if (!indexBufferStage.buffer)
    {
        throw std::runtime_error("SYSTEM: Failed to create index staging buffer");
    }
    
    // device local buffers
    get::gpu_buffer vertexBuffer = create_buffer(VK_BUFFER_USAGE_TRANSFER_DST_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, vertexBufferBytes, false, VMA_MEMORY_USAGE_AUTO);
    if (!vertexBuffer.buffer)
    {
        throw std::runtime_error("SYSTEM: Failed to create vertex buffer");
    }

    _vertex_buffer_id = add_buffer(vertexBuffer);
    _vma->copy_buffer_data(vertexBufferStage, 0, _vertices.data(), _vertices.size() * sizeof(get::vertex));

    get::gpu_buffer indexBuffer = create_buffer(VK_BUFFER_USAGE_INDEX_BUFFER_BIT | VK_BUFFER_USAGE_TRANSFER_DST_BIT, indexBufferBytes, false, VMA_MEMORY_USAGE_AUTO);

    if (!indexBuffer.buffer)
    {
        throw std::runtime_error("SYSTEM: Failed to create index buffer");
    }

    _index_buffer_id = add_buffer(indexBuffer);
    _vma->copy_buffer_data(indexBufferStage, 0, _indices.data(), _vertices.size() * sizeof(u32));

    VkCommandBuffer geoCmdBuffer = start_transient_command_buffer();
    VkBufferCopy buffCopyVerts 
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = vertexBufferBytes
    };

    vkCmdCopyBuffer(geoCmdBuffer, vertexBufferStage.buffer, vertexBuffer.buffer, 1, &buffCopyVerts);
    VkBufferCopy buffCopyIndices 
    {
        .srcOffset = 0,
        .dstOffset = 0,
        .size = indexBufferBytes
    };

    vkCmdCopyBuffer(geoCmdBuffer, indexBufferStage.buffer, indexBuffer.buffer, 1, &buffCopyIndices);
    submit_transient_command_buffer(geoCmdBuffer);

    _vma->destroy(vertexBufferStage.buffer, vertexBufferStage.allocation);
    _vma->destroy(indexBufferStage.buffer, indexBufferStage.allocation);
    
    _descriptor_set->update_texture_descriptors(_textures, _samplers, _gpu_images);

    const size_t materialDataBytes = _materials.size() * sizeof(get::material);

    get::gpu_buffer materialBuffer = create_buffer(
            VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT,
            materialDataBytes,
            true,
            VMA_MEMORY_USAGE_AUTO);

    if (!materialBuffer.buffer)
    {
        throw std::runtime_error("SYSTEM: Failed to create material buffer");
    }

    _material_buffer_id = add_buffer(materialBuffer);
    _vma->copy_buffer_data(materialBuffer, 0, _materials.data(), materialDataBytes);
}


void application::create_indirect_buffers()
{
    for (auto& res : _frame_resources)
    {
        const size_t indirectBufferByteSize = _node_world->max_nodes() * sizeof(VkDrawIndexedIndirectCommand);
        res.indirect_draw_buffer = create_buffer(VK_BUFFER_USAGE_INDIRECT_BUFFER_BIT, indirectBufferByteSize, true, VMA_MEMORY_USAGE_AUTO);

        void* indirectBufferPtr = nullptr;
        if (vmaMapMemory(_vma->get_allocator(), res.indirect_draw_buffer.allocation, &indirectBufferPtr) != VK_SUCCESS)
        {
            throw std::runtime_error("SYSTEM: Failed to map indirect draw buffer");
        }

        res.indirect_draw_ptr = reinterpret_cast<VkDrawIndexedIndirectCommand*>(indirectBufferPtr);

        const size_t renderItemByteSize = _node_world->max_nodes() * sizeof(get::render_item);
        res.render_item_buffer = create_buffer(VK_BUFFER_USAGE_STORAGE_BUFFER_BIT | VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT, renderItemByteSize, true, VMA_MEMORY_USAGE_AUTO);
        
        void* renderItemBufferPtr = nullptr;
        if (vmaMapMemory(_vma->get_allocator(), res.render_item_buffer.allocation, &renderItemBufferPtr) != VK_SUCCESS)
        {
            throw std::runtime_error("SYSTEM: Failed to map render item buffer");
        }

        res.render_item_ptr = reinterpret_cast<get::render_item*>(renderItemBufferPtr);
    }
}

u32 application::add_buffer(const get::gpu_buffer& buffer)
{
    _buffers.push_back(buffer);
    u32 bufferID = _buffers.size();
    return bufferID;
}

void application::load_gltf(const std::string& filepath)
{
    if (!std::filesystem::exists(filepath))
    {
        std::print("SYSTEM: File doesn't exist: {0}\n", filepath);
        return;
    }

    std::print("SYSTEM: Loading GLTF: {}\n", filepath);

    tg3_model model;
    tg3_parse_options options;
    tg3_error_stack errors;

    tg3_parse_options_init(&options);
    tg3_error_stack_init(&errors);
    tg3_error_code parseResult = tg3_parse_file(&model, &errors, filepath.c_str(), filepath.size(), &options);

    if (parseResult != TG3_OK)
    {
        std::println("SYSTEM: ERROR parsing GLTF file, errors found:");
        for (u32 i = 0; i < errors.count; i++)
        {
            std::println("{0}", errors.entries[i].message);
        }

        tg3_error_stack_free(&errors);
        return;
    }

    tg3_error_stack_free(&errors);
    
    std::filesystem::path imageDir = std::filesystem::path(filepath).parent_path();
    std::vector<get::image> images = load_images(model, imageDir); // load images into RAM
    std::vector<u32> imageIDs = upload_images(images); // load images into VRAM

    // free images after uploading into VRAM
    for (const get::image& image : images)
    {
        stbi_image_free(image.data);
    }

    std::vector<u32> samplerIDs = load_samplers(model);
    std::vector<u32> textureIDs = load_textures(model, imageIDs, samplerIDs);
    std::vector<u32> materialIDs = load_materials(model, textureIDs);
    std::vector<u32> meshIDs = load_meshes(model, materialIDs);

    const tg3_scene* scene = &model.scenes[model.default_scene != -1 ? model.default_scene : 0];

    for (size_t i = 0; i < scene->nodes_count; i++)
    {
        u32 nodeID = import_node(*_node_world, model, scene->nodes[i], 0, _last_root_node_id, meshIDs);

        if (!_root_node_id)
        {
            _root_node_id = nodeID;
            _last_root_node_id = nodeID;
        }
        else 
        {
            _last_root_node_id = nodeID;
        }
    }

    tg3_model_free(&model);
    std::println("SYSTEM: GLTF model loaded successfully");
}


void application::update_texture_descriptors() const
{
    std::vector<VkDescriptorImageInfo> imageDescriptors;
    imageDescriptors.reserve(_textures.size());

    for (const auto& texture : _textures)
    {
        imageDescriptors.push_back(
                {
                    .sampler = _samplers[texture.sampler_id - 1],
                    .imageView = _gpu_images[texture.image_id - 1].image_view,
                    .imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL
                });
    }

    VkWriteDescriptorSet descSetWrite
    {
        .sType = VK_STRUCTURE_TYPE_WRITE_DESCRIPTOR_SET,
        .dstSet = _descriptor_set->get_global_descriptor_set(),
        .dstBinding = 0,
        .dstArrayElement = 0,
        .descriptorCount = static_cast<u32>(imageDescriptors.size()),
        .descriptorType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER,
        .pImageInfo = imageDescriptors.data()
    };

    vkUpdateDescriptorSets(_vulkan_device->get_device(), 1, &descSetWrite, 0, nullptr);
}

std::vector<u32> application::load_samplers(const tg3_model& model)
{
    i32 noFilter = -1;
    std::vector<u32> samplerIDs(model.samplers_count);

    for (u32 i = 0; i < samplerIDs.size(); i++)
    {
        const tg3_sampler& sampler = model.samplers[i];
        static const std::unordered_map<i32, std::tuple<VkFilter, VkSamplerMipmapMode, float>> filterMap
        {
            { TG3_TEXTURE_FILTER_NEAREST, { VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.25f }},
            { TG3_TEXTURE_FILTER_LINEAR, { VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, 0.25f }},
            { TG3_TEXTURE_FILTER_LINEAR_MIPMAP_LINEAR, { VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_LOD_CLAMP_NONE }},
            { TG3_TEXTURE_FILTER_NEAREST_MIPMAP_NEAREST, { VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_LOD_CLAMP_NONE }},
            { TG3_TEXTURE_FILTER_NEAREST_MIPMAP_LINEAR, { VK_FILTER_NEAREST, VK_SAMPLER_MIPMAP_MODE_LINEAR, VK_LOD_CLAMP_NONE }},
            { TG3_TEXTURE_FILTER_LINEAR_MIPMAP_NEAREST, { VK_FILTER_LINEAR, VK_SAMPLER_MIPMAP_MODE_NEAREST, VK_LOD_CLAMP_NONE }},
        };

        static const std::unordered_map<u32, VkSamplerAddressMode> wrapMap
        {
            { TG3_TEXTURE_WRAP_REPEAT, VK_SAMPLER_ADDRESS_MODE_REPEAT },
            { TG3_TEXTURE_WRAP_CLAMP_TO_EDGE, VK_SAMPLER_ADDRESS_MODE_CLAMP_TO_EDGE },
            { TG3_TEXTURE_WRAP_MIRRORED_REPEAT, VK_SAMPLER_ADDRESS_MODE_MIRRORED_REPEAT }
        };


        VkSamplerCreateInfo samplerInfo
        {
            .sType = VK_STRUCTURE_TYPE_SAMPLER_CREATE_INFO,
            .magFilter = (sampler.mag_filter == noFilter) ? VK_FILTER_LINEAR : std::get<0>(filterMap.at(sampler.mag_filter)),
            .minFilter = (sampler.min_filter == noFilter) ? VK_FILTER_LINEAR : std::get<0>(filterMap.at(sampler.min_filter)),
            .mipmapMode = (sampler.min_filter == noFilter) ? VK_SAMPLER_MIPMAP_MODE_LINEAR : std::get<1>(filterMap.at(sampler.min_filter)),
            .addressModeU = (sampler.wrap_s == noFilter) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : wrapMap.at(sampler.wrap_s),
            .addressModeV = (sampler.wrap_t == noFilter) ? VK_SAMPLER_ADDRESS_MODE_REPEAT : wrapMap.at(sampler.wrap_t),
            .addressModeW = VK_SAMPLER_ADDRESS_MODE_REPEAT,
            .compareEnable = VK_FALSE,
            .minLod = 0.0f,
            .maxLod = (sampler.min_filter == noFilter) ? VK_LOD_CLAMP_NONE : std::get<2>(filterMap.at(sampler.min_filter))
        };

        VkSampler vulkanSampler = nullptr;
        if (vkCreateSampler(_vulkan_device->get_device(), &samplerInfo, nullptr, &vulkanSampler) != VK_SUCCESS)
        {
            std::println("Unable to create texture sampler");
            samplerIDs[i] = _textures[0].sampler_id;
        }
        else 
        {
            _samplers.push_back(vulkanSampler);
            samplerIDs[i] = _samplers.size();
        }
    }
    
    return samplerIDs;
}


std::vector<u32> application::load_textures(const tg3_model& model, const std::vector<u32>& images, const std::vector<u32>& samplers)
{
    assert(_textures.size() + model.textures_count <= MAX_TEXTURES && "Exceeding max texture count");
    std::vector<u32> textureIDs(model.textures_count);
    for (size_t i = 0; i < model.textures_count; i++)
    {
        const tg3_texture& tex = model.textures[i];
        u32 samplerID = (tex.sampler >= 0) ? samplers[tex.sampler] : _fallback_image_id;
        _textures.push_back(get::texture { .image_id = images[tex.source], .sampler_id = samplerID });
        textureIDs[i] = static_cast<u32>(_textures.size());    }

    return textureIDs;
}

std::vector<u32> application::load_materials(const tg3_model& model, const std::vector<u32>& textures)
{
    i32 noTexture = -1;
    std::vector<u32> materialIDs(model.materials_count);

    for (size_t i = 0; i < model.materials_count; i++)
    {
        const tg3_material* mat = &model.materials[i];
        _materials.push_back(get::material 
            {
                .base_color = glm::vec4(
                        mat->pbr_metallic_roughness.base_color_factor[0],
                        mat->pbr_metallic_roughness.base_color_factor[1],
                        mat->pbr_metallic_roughness.base_color_factor[2],
                        mat->pbr_metallic_roughness.base_color_factor[3]),
                .texture_id = mat->pbr_metallic_roughness.base_color_texture.index != noTexture
                    ? textures[mat->pbr_metallic_roughness.base_color_texture.index] - 1 
                    : 0
            });

        materialIDs[i] = _materials.size();
    }
    return materialIDs;
}


std::vector<u32> application::load_meshes(const tg3_model& model, const std::vector<u32>& materials)
{
    std::vector<u32> meshIDs(model.meshes_count);
    for (size_t i = 0; i < model.meshes_count; i++)
    {
        get::mesh mesh{};
        const tg3_mesh* tg3Mesh = &model.meshes[i];

        mesh.name = tg3Mesh->name.data != nullptr ? tg3Mesh->name.data : "No name";

        auto write_attribute = [this, &model]<typename T>(T get::vertex::* member, const tg3_str_int_pair* attr)
        {
            const tg3_accessor* accessor = &model.accessors[attr->value];
            const tg3_buffer_view* bufferView = &model.buffer_views[accessor->buffer_view];
            const tg3_buffer* buffer = &model.buffers[bufferView->buffer];
            const size_t bufferOffset = bufferView->byte_offset + accessor->byte_offset;
            const size_t stride = bufferView->byte_stride != 0 ? bufferView->byte_stride : sizeof(T);

            for (u64 id = 0; id < accessor->count; id++)
            {
                const size_t elementOffset = bufferOffset + id * stride;
                const float* data = reinterpret_cast<const float*>(buffer->data.data + elementOffset);

                if constexpr (std::is_same<T, glm::vec3>())
                {
                    _vertices[_vert_offset + id].*member = glm::vec3(data[0], data[1], data[2]);
                }
                else if constexpr (std::is_same<T, glm::vec2>()) 
                {
                    _vertices[_vert_offset + id].*member = glm::vec2(data[0], data[1]);
                }
            }
        };

        mesh.sub_meshes.resize(tg3Mesh->primitives_count);

        for (size_t s = 0; s < tg3Mesh->primitives_count; s++)
        {
            const tg3_primitive* primitive = &tg3Mesh->primitives[s];
            // mesh.sub_meshes[s].material_id = materials[primitive->material];
            
            if (primitive->material >= 0)
            {
                assert(primitive->material < materials.size());
                mesh.sub_meshes[s].material_id = materials[primitive->material];
            }
            else
            {
                mesh.sub_meshes[s].material_id = 0;
            }

            mesh.sub_meshes[s].vertex_start = _vert_offset;

            for (size_t a = 0; a < primitive->attributes_count; a++)
            {
                const tg3_str_int_pair* attr = &primitive->attributes[a];
                if (strcmp(attr->key.data, "POSITION") == 0)
                {
                    const tg3_accessor* accessor = &model.accessors[attr->value];
                    assert(accessor->type == TG3_TYPE_VEC3 && accessor->component_type == TG3_COMPONENT_TYPE_FLOAT);
                    assert(_vert_offset + accessor->count <= _vertices.size() && "SYSTEM: Not enough space to load verticies");

                    mesh.sub_meshes[s].vertex_count = accessor->count;
                    write_attribute(&get::vertex::position, attr);
                }
                else if (strcmp(attr->key.data, "NORMAL") == 0)
                {
                    const tg3_accessor* accessor = &model.accessors[attr->value];
                    assert(accessor->type == TG3_TYPE_VEC3 && accessor->component_type == TG3_COMPONENT_TYPE_FLOAT);
                    write_attribute(&get::vertex::normal, attr);
                }
                else if (strcmp(attr->key.data, "COLOR_0") == 0)
                {
                    const tg3_accessor* accessor = &model.accessors[attr->value];
                    assert(accessor->type == TG3_TYPE_VEC3 || accessor->type == TG3_TYPE_VEC4);
                    assert(accessor->component_type == TG3_COMPONENT_TYPE_FLOAT);
                    write_attribute(&get::vertex::color, attr);
                }
                else if (strcmp(attr->key.data, "TEXCOORD_0") == 0)
                {
                    const tg3_accessor* accessor = &model.accessors[attr->value];
                    assert(accessor->type == TG3_TYPE_VEC2 && accessor->component_type == TG3_COMPONENT_TYPE_FLOAT);
                    write_attribute(&get::vertex::uv, attr);
                }
            } 

            _vert_offset += mesh.sub_meshes[s].vertex_count;

            if (primitive->indices != -1)
            {
                const tg3_accessor* accessor = &model.accessors[primitive->indices];
                const tg3_buffer_view* bufferView = &model.buffer_views[accessor->buffer_view];
                const tg3_buffer* buffer = &model.buffers[bufferView->buffer];
                assert(_id_offset + accessor->count <= _indices.size() && "Not enough space for indices");

                mesh.sub_meshes[s].index_start = _id_offset;
                mesh.sub_meshes[s].index_count = accessor->count;

                if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_INT)
                {
                    const u32* bufferData = reinterpret_cast<const u32*>(buffer->data.data + bufferView->byte_offset + accessor->byte_offset);
                    memcpy(&_indices[_id_offset], bufferData, accessor->count * sizeof(u32));
                }
                else if (accessor->component_type == TG3_COMPONENT_TYPE_UNSIGNED_SHORT)
                {
                    const u16* bufferData = reinterpret_cast<const u16*>(buffer->data.data + bufferView->byte_offset + accessor->byte_offset); 

                    for (u64 id = 0; id < accessor->count; id++)
                    {
                        _indices[_id_offset + id] = static_cast<u32>(bufferData[id]);
                    }
                }

                _id_offset += mesh.sub_meshes[s].index_count;
            }
        }

        _meshes.push_back(std::move(mesh));
        meshIDs[i] = _meshes.size();
    } 

    return meshIDs;
}


u32 application::import_node(get::node_world& nodeWorld, const tg3_model& model, i32 nodeIndex, u32 parentId, u32 prevSiblingId, std::vector<u32>& meshIds)
{
    const tg3_node& tg3node = model.nodes[nodeIndex];

    auto [node, nodeID] = nodeWorld.create_node();
    node.data().parent_id = parentId;

    if (tg3node.has_matrix)
    {
        glm::mat4 transform(1);
        float* transformPtr = glm::value_ptr(transform);
        for (i32 i = 0; i < 16; i++)
        {
            transformPtr[i] = static_cast<f32>(tg3node.matrix[i]);
        }
        node.set_transform(transform);
    }
    else 
    {
        glm::vec3 translation(tg3node.translation[0], tg3node.translation[1], tg3node.translation[2]);
        glm::quat rotation(tg3node.rotation[3], tg3node.rotation[0], tg3node.rotation[1], tg3node.rotation[2]);
        glm::vec3 scale(tg3node.scale[0], tg3node.scale[1], tg3node.scale[2]);

        node.set_translation(translation);
        node.set_rotation(rotation);
        node.set_scale(scale);
    }

    if (tg3node.mesh != -1)
    {
        node.data().mesh_id = meshIds[tg3node.mesh];
    }

    if (prevSiblingId)
    {
        auto& n = nodeWorld.get_node(prevSiblingId);
        n.data().next_sibling_id = nodeID;
    }

    u32 lastChildID = 0;

    for (size_t i = 0; i < tg3node.children_count; i++)
    {
        i32 childIndex = tg3node.children[i];
        lastChildID = import_node(nodeWorld, model, childIndex, nodeID, lastChildID, meshIds);

        if (!node.data().first_child_id)
        {
            node.data().first_child_id = lastChildID;
        }
    }

    return nodeID;
}

std::vector<u32> application::upload_images(const std::vector<get::image>& images)
{
    VkCommandBuffer commandBuffer = start_transient_command_buffer();

    std::vector<get::gpu_buffer> stagingBuffers;
    stagingBuffers.reserve(images.size());

    i32 targetColorChannels = 4;
    std::vector<u32> imageIDs(images.size());
    for (size_t i = 0; i < images.size(); i++)
    {
        const get::image& image = images[i];

        if (image.data)
        {
            auto [imageID, stagingTextureBuffer] = create_image(commandBuffer, image.data, image.width, image.height, targetColorChannels);
            imageIDs[i] = imageID;
            stagingBuffers.push_back(stagingTextureBuffer);
        }
        else
        {
            imageIDs[i] = _fallback_image_id;
        }
    }

    submit_transient_command_buffer(commandBuffer);

    for (auto& stageBuffer : stagingBuffers)
    {
        _vma->destroy(stageBuffer.buffer, stageBuffer.allocation);
    }

    return imageIDs;
}


std::vector<get::image> application::load_images(const tg3_model& model, const std::filesystem::path& path)
{
    i32 targetColorChannels = 4;
    std::vector<get::image> images(model.images_count);

    for (u32 i = 0; i < model.images_count; i++)
    {
        get::image& img = images[i];
        std::filesystem::path imagePath = path / model.images[i].uri.data;
        std::print("SYSTEM: Loading image {}/{}: {}\n", i + 1, model.images_count, model.images[i].uri.data);

        img.data = stbi_load(imagePath.string().c_str(), &img.width, &img.height, &img.channels, targetColorChannels);

        if (!img.data)
        {
            throw std::runtime_error("SYSTEM: Failed to load image: " + imagePath.string());
        }
    }

    return images;
}

void application::submit_transient_command_buffer(VkCommandBuffer commandBuffer)
{
    vkEndCommandBuffer(commandBuffer);

    VkSubmitInfo submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO,
        .commandBufferCount = 1,
        .pCommandBuffers = &commandBuffer
    };

    vkQueueSubmit(_vulkan_device->get_queue(), 1, &submitInfo, nullptr);
    vkQueueWaitIdle(_vulkan_device->get_queue());
    vkFreeCommandBuffers(_vulkan_device->get_device(), _command_pool->get_transient_command_pool(), 1, &commandBuffer);
}

std::pair<u32, get::gpu_buffer> application::create_image(VkCommandBuffer commandBuffer, unsigned char* imageData, u32 width, u32 height, i32 channels)
{
    VkFormat imageFormat = VK_FORMAT_R8G8B8A8_SRGB;
    VkImageCreateInfo imageInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_CREATE_INFO,
        .imageType = VK_IMAGE_TYPE_2D,
        .format = imageFormat,
        .extent = { .width = width, .height = height, .depth = 1 },
        .mipLevels = 1,
        .arrayLayers = 1,
        .samples = VK_SAMPLE_COUNT_1_BIT,
        .tiling = VK_IMAGE_TILING_OPTIMAL,
        .usage = VK_IMAGE_USAGE_TRANSFER_DST_BIT | VK_IMAGE_USAGE_SAMPLED_BIT,
        .initialLayout = VK_IMAGE_LAYOUT_UNDEFINED
    };

    VmaAllocationCreateInfo allocInfo { .usage = VMA_MEMORY_USAGE_AUTO };
    
    get::gpu_image gpuImage = _vma->create_image(&imageInfo, &allocInfo); 

    VkImageViewCreateInfo imageViewInfo
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_VIEW_CREATE_INFO,
        .image = gpuImage.image,
        .viewType = VK_IMAGE_VIEW_TYPE_2D,
        .format = imageFormat,
        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .levelCount = 1,
            .layerCount = 1
        }
    };

    VkResult res = vkCreateImageView(_vulkan_device->get_device(), &imageViewInfo, nullptr, &gpuImage.image_view);

    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create image view");
    }

    VkImageMemoryBarrier2 transferBarrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_NONE,
        .srcAccessMask = VK_ACCESS_2_NONE,
        .dstStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
        .dstAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
        .newLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .image = gpuImage.image,
        .subresourceRange
        {   
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    VkDependencyInfo transferDepInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &transferBarrier
    };

    vkCmdPipelineBarrier2(commandBuffer, &transferDepInfo);

    const size_t byteSize = width * height * channels;
    get::gpu_buffer stageBuffer = create_buffer(VK_BUFFER_USAGE_TRANSFER_SRC_BIT, byteSize, true, VMA_MEMORY_USAGE_AUTO_PREFER_HOST);
    _vma->copy_buffer_data(stageBuffer, 0, imageData, byteSize);

    VkBufferImageCopy bufferImageCopy
    {
        .imageSubresource 
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .mipLevel = 0,
            .baseArrayLayer = 0,
            .layerCount = 1
        },
        .imageExtent 
        {
            .width = width,
            .height = height,
            .depth = 1
        }
    };

    vkCmdCopyBufferToImage(commandBuffer, stageBuffer.buffer, gpuImage.image, VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL, 1, &bufferImageCopy);
    VkImageMemoryBarrier2 shaderReadBarrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COPY_BIT,
        .srcAccessMask = VK_ACCESS_2_TRANSFER_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_FRAGMENT_SHADER_BIT,
        .dstAccessMask = VK_ACCESS_SHADER_READ_BIT,
        .oldLayout = VK_IMAGE_LAYOUT_TRANSFER_DST_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL,
        .image = gpuImage.image,
        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    VkDependencyInfo shaderReadDepInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &shaderReadBarrier
    };

    vkCmdPipelineBarrier2(commandBuffer, &shaderReadDepInfo);
    
    _gpu_images.push_back(gpuImage);
    const u32 imageID = _gpu_images.size();
    return { imageID, stageBuffer} ;
    
}

get::gpu_buffer application::create_buffer(VkBufferUsageFlags usage, size_t byteSize, bool mappable, VmaMemoryUsage memoryUsage)
{
    VkBufferCreateInfo bufferInfo
    {
        .sType = VK_STRUCTURE_TYPE_BUFFER_CREATE_INFO,
        .size = byteSize,
        .usage = usage,
        .sharingMode = VK_SHARING_MODE_EXCLUSIVE
    };

    VmaAllocationCreateInfo allocInfo
    {
        .flags = mappable ? VMA_ALLOCATION_CREATE_HOST_ACCESS_SEQUENTIAL_WRITE_BIT: 0u,
        .usage = memoryUsage,
    };

    
   get::gpu_buffer gpuBuffer = _vma->create_buffer(&bufferInfo, &allocInfo);

    if (usage & VK_BUFFER_USAGE_SHADER_DEVICE_ADDRESS_BIT)
    {
        VkBufferDeviceAddressInfo vertBdaInfo
        {
            .sType = VK_STRUCTURE_TYPE_BUFFER_DEVICE_ADDRESS_INFO,
            .buffer = gpuBuffer.buffer
        };

        gpuBuffer.device_adress = vkGetBufferDeviceAddress(_vulkan_device->get_device(), &vertBdaInfo);
    }

    return gpuBuffer;


}

VkCommandBuffer application::start_transient_command_buffer()
{
    VkCommandBufferAllocateInfo allocInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO,
        .commandPool = _command_pool->get_transient_command_pool(),
        .level = VK_COMMAND_BUFFER_LEVEL_PRIMARY,
        .commandBufferCount = 1,
    };

    VkCommandBuffer commandBuffer = nullptr;
    VkResult res = vkAllocateCommandBuffers(_vulkan_device->get_device(), &allocInfo, &commandBuffer);
    if (res != VK_SUCCESS)
    {
        throw std::runtime_error("SYSTEM: Failed to create command buffer");
        return nullptr;
    }

    VkCommandBufferBeginInfo beginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    res = vkBeginCommandBuffer(commandBuffer, &beginInfo);
    if (res != VK_SUCCESS)
    {
        vkFreeCommandBuffers(_vulkan_device->get_device(), _command_pool->get_transient_command_pool(), 1, &commandBuffer);
        throw std::runtime_error("SYSTEM: Failed to create command buffer");
        return nullptr;
    }

    return commandBuffer;
}

void application::run()
{
    std::println("{0}", "SYSTEM: Application is running");

    int currentWidth = 0;
    int currentHeight = 0;
    int lastWidth = 0;
    int lastHeight = 0;
    auto window = _window->get_current_window();

    while (!glfwWindowShouldClose(window))
    {
        get::frame_time::update();
        glfwGetFramebufferSize(window, &currentWidth, &currentHeight);

        while (currentWidth == 0 || currentHeight == 0)
        {
            glfwGetFramebufferSize(window, &currentWidth, &currentHeight);
            glfwWaitEvents();
        }
        
        if (currentWidth != lastWidth || currentHeight != lastHeight)
        {
            _recreate_swapchain = true;
            lastWidth = currentWidth;
            lastHeight = currentHeight;
        }
        
        _main_camera->update(currentWidth, currentHeight);

        render(currentWidth, currentHeight);
        glfwPollEvents();
    }
}

void application::render(int width, int height)
{
    if (_recreate_swapchain)
    {
        vkDeviceWaitIdle(_vulkan_device->get_device());
        _swapchain->destroy();
        _swapchain->create(width, height);
        _depth_buffer->destroy();
        _depth_buffer->create(width, height);

        _recreate_swapchain = false;
    }

    const u32 frameResIndex = _frame_index++ % _max_frames_in_flight;
    const u64 signalValue = ++_next_signal_value;
    const u64 waitValue = (signalValue > _max_frames_in_flight) ? (signalValue - _max_frames_in_flight) : 0;

    auto semaphore = _semaphore->get_semaphore();
    VkSemaphoreWaitInfo waitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_WAIT_INFO,
        .semaphoreCount = 1,
        .pSemaphores = &semaphore,
        .pValues = &waitValue,
    };

    vkWaitSemaphores(_vulkan_device->get_device(), &waitInfo, UINT64_MAX);
    
    // start working on the frame
    auto& resource = _frame_resources[frameResIndex];
    vkResetCommandPool(_vulkan_device->get_device(), resource.command_pool, 0);

    VkSemaphore imageAcquireSemaphore = _frame_resources[frameResIndex].image_acquired_semaphore;

    u32 imageIndex = 0;
    VkResult res = vkAcquireNextImageKHR(_vulkan_device->get_device(),
                                            _swapchain->get_swapchain(),
                                            UINT64_MAX,
                                            imageAcquireSemaphore,
                                            VK_NULL_HANDLE, &imageIndex);

    if (res == VK_ERROR_OUT_OF_DATE_KHR)
    {
        _recreate_swapchain = true;
        return;
    }
    else if (res == VK_SUBOPTIMAL_KHR)
    {
        _recreate_swapchain = true;
    }

    _node_render_stack.clear();

    u32 nodeID = _root_node_id;

    while (nodeID)
    {
        get::node& node = _node_world->get_node(nodeID);
        _node_render_stack.push_back({ &node, glm::mat4(1.0f) });
        nodeID = node.data().next_sibling_id;
    }

    u32 drawIndex = 0;
    while (!_node_render_stack.empty())
    {
        auto [node, parentTransform] = _node_render_stack.back();
        _node_render_stack.pop_back();
        glm::mat4 matWorld = parentTransform * node->get_transform();

        if (node->data().mesh_id)
        {
            get::mesh& mesh = _meshes[node->data().mesh_id - 1];
            for (auto& subMesh : mesh.sub_meshes)
            {
                resource.indirect_draw_ptr[drawIndex] = VkDrawIndexedIndirectCommand
                {
                    .indexCount = static_cast<u32>(subMesh.index_count),
                    .instanceCount = 1,
                    .firstIndex = static_cast<u32>(subMesh.index_start),
                    .vertexOffset = static_cast<i32>(subMesh.vertex_start),
                    .firstInstance = drawIndex
                };

                resource.render_item_ptr[drawIndex] = get::render_item 
                {
                    .wvp = _main_camera->get_view_projection_matrix() * matWorld,
                    .world_matrix = matWorld,
                    .material_index = subMesh.material_id - 1
                };
                drawIndex++;
            }
        }

        u32 childNodeID = node->data().first_child_id;
        while (childNodeID)
        {
            get::node& child = _node_world->get_node(childNodeID);
            _node_render_stack.push_back({ &child, matWorld });
            childNodeID = child.data().next_sibling_id;
        }
    }

    // begin recording commands
    VkCommandBufferBeginInfo commandBeginInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO,
        .flags = VK_COMMAND_BUFFER_USAGE_ONE_TIME_SUBMIT_BIT,
    };

    vkBeginCommandBuffer(resource.command_buffer, &commandBeginInfo);
    
    std::vector<VkImageMemoryBarrier2> layoutBarriers
    {
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
            .dstAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
            .image = _swapchain->get_swapchain_images()[imageIndex],
            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        },
        {
            .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
            .srcStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT,
            .srcAccessMask = 0,
            .dstStageMask = VK_PIPELINE_STAGE_2_EARLY_FRAGMENT_TESTS_BIT | VK_PIPELINE_STAGE_2_LATE_FRAGMENT_TESTS_BIT,
            .dstAccessMask = VK_ACCESS_2_DEPTH_STENCIL_ATTACHMENT_WRITE_BIT,
            .oldLayout = VK_IMAGE_LAYOUT_UNDEFINED,
            .newLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
            .image = _depth_buffer->get_image(),
            .subresourceRange
            {
                .aspectMask = VK_IMAGE_ASPECT_DEPTH_BIT,
                .baseMipLevel = 0,
                .levelCount = 1,
                .baseArrayLayer = 0,
                .layerCount = 1,
            }
        }
    };

    VkDependencyInfo dependencyInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = static_cast<u32>(layoutBarriers.size()),
        .pImageMemoryBarriers = layoutBarriers.data(),
    };
    
    vkCmdPipelineBarrier2(resource.command_buffer, &dependencyInfo);
    
    // setup attachments
    VkRenderingAttachmentInfo colorAttachInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = _swapchain->get_swapchain_image_views()[imageIndex],
        .imageLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_STORE,
        .clearValue 
        { 
            .color = {{ 0.01f, 0.01f, 0.01f, 1 }}
        }
    };

    VkRenderingAttachmentInfo depthAttachInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_ATTACHMENT_INFO,
        .imageView = _depth_buffer->get_image_view(),
        .imageLayout = VK_IMAGE_LAYOUT_DEPTH_ATTACHMENT_OPTIMAL,
        .loadOp = VK_ATTACHMENT_LOAD_OP_CLEAR,
        .storeOp = VK_ATTACHMENT_STORE_OP_DONT_CARE,
        .clearValue 
        {
            .depthStencil { 1.0f, 0 }
        }
    };

    VkRenderingInfo renderingInfo
    {
        .sType = VK_STRUCTURE_TYPE_RENDERING_INFO,
        .renderArea 
        {
            .offset { .x = 0, .y = 0 },
            .extent
            {
                .width = static_cast<u32>(width),
                .height = static_cast<u32>(height)
            }
        },
        .layerCount = 1,
        .colorAttachmentCount = 1,
        .pColorAttachments = &colorAttachInfo,
        .pDepthAttachment = &depthAttachInfo
    };

    
    auto gds = _descriptor_set->get_global_descriptor_set();  
    vkCmdBindDescriptorSets(
            resource.command_buffer,
            VK_PIPELINE_BIND_POINT_GRAPHICS,
            _vulkan_pipeline->get_layout(),
            0,
            1,
            &gds,
            0,
            nullptr);
    get::frame_constants frameConstants {};
    get::gpu_buffer& vertBuffer = _buffers[_vertex_buffer_id - 1];
    get::gpu_buffer& materialBuffer = _buffers[_material_buffer_id - 1];

    frameConstants.vertex_buffer_address = vertBuffer.device_adress;
    frameConstants.material_buffer_address = materialBuffer.device_adress;
    frameConstants.render_items_buffer_address = resource.render_item_buffer.device_adress;
    vkCmdPushConstants(
            resource.command_buffer,
            _vulkan_pipeline->get_layout(), 
            VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT, 
            0, 
            sizeof(get::frame_constants), 
            &frameConstants);

    get::gpu_buffer& idxBuffer = _buffers[_index_buffer_id - 1];
    vkCmdBindIndexBuffer(resource.command_buffer, idxBuffer.buffer, 0, VK_INDEX_TYPE_UINT32);

    // begin dynamic rendering
    vkCmdBeginRendering(resource.command_buffer, &renderingInfo);
    {
        VkViewport viewport
        {
            .x = 0, 
            .y = static_cast<f32>(height),
            .width = static_cast<f32>(width),
            .height = -static_cast<f32>(height),
            .minDepth = 0,
            .maxDepth = 1
        };
        vkCmdSetViewport(resource.command_buffer, 0, 1, &viewport);

        VkRect2D scissor
        {
            .offset { .x = 0, .y = 0},
            .extent 
            { 
                .width = static_cast<u32>(width),
                .height = static_cast<u32>(height),
            }
        };

        vkCmdSetScissor(resource.command_buffer, 0, 1, &scissor);
        vkCmdBindPipeline(resource.command_buffer, VK_PIPELINE_BIND_POINT_GRAPHICS, _vulkan_pipeline->get_pipeline());
        // vkCmdDraw(resource.command_buffer, 3, 1, 0, 0);
        vkCmdDrawIndexedIndirect(resource.command_buffer, resource.indirect_draw_buffer.buffer, 0, drawIndex, sizeof(VkDrawIndexedIndirectCommand));
    }
    vkCmdEndRendering(resource.command_buffer);
    
    // change memory layout of the swapchain to display the image
    VkImageMemoryBarrier2 presentLayoutBarrier
    {
        .sType = VK_STRUCTURE_TYPE_IMAGE_MEMORY_BARRIER_2,
        .srcStageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT,
        .srcAccessMask = VK_ACCESS_2_COLOR_ATTACHMENT_WRITE_BIT,
        .dstStageMask = VK_PIPELINE_STAGE_2_NONE,
        .dstAccessMask = 0,
        .oldLayout = VK_IMAGE_LAYOUT_COLOR_ATTACHMENT_OPTIMAL,
        .newLayout = VK_IMAGE_LAYOUT_PRESENT_SRC_KHR,
        .image = _swapchain->get_swapchain_images()[imageIndex],
        .subresourceRange
        {
            .aspectMask = VK_IMAGE_ASPECT_COLOR_BIT,
            .baseMipLevel = 0,
            .levelCount = 1,
            .baseArrayLayer = 0,
            .layerCount = 1,
        }
    };

    VkDependencyInfo presentDependencyInfo
    {
        .sType = VK_STRUCTURE_TYPE_DEPENDENCY_INFO,
        .imageMemoryBarrierCount = 1,
        .pImageMemoryBarriers = &presentLayoutBarrier
    };

    vkCmdPipelineBarrier2(resource.command_buffer, &presentDependencyInfo);
    vkEndCommandBuffer(resource.command_buffer);
    
    // ensure swapchain image is available to start color output
    VkSemaphoreSubmitInfo imageAcquireWaitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
        .semaphore = imageAcquireSemaphore,
        .stageMask = VK_PIPELINE_STAGE_2_COLOR_ATTACHMENT_OUTPUT_BIT
    };

    std::vector<VkSemaphoreSubmitInfo> semaphoreSignals
    {
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = _swapchain->get_render_complete_semaphores()[imageIndex],
            .stageMask = VK_PIPELINE_STAGE_2_ALL_GRAPHICS_BIT
        },
        {
            .sType = VK_STRUCTURE_TYPE_SEMAPHORE_SUBMIT_INFO,
            .semaphore = _semaphore->get_semaphore(),
            .value = signalValue,
            .stageMask = VK_PIPELINE_STAGE_2_ALL_COMMANDS_BIT
        }
    };

    VkCommandBufferSubmitInfo cmdSubmitInfo
    {
        .sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_SUBMIT_INFO,
        .commandBuffer = resource.command_buffer,
    };

    VkSubmitInfo2 submitInfo
    {
        .sType = VK_STRUCTURE_TYPE_SUBMIT_INFO_2,
        .waitSemaphoreInfoCount = 1,
        .pWaitSemaphoreInfos = &imageAcquireWaitInfo,
        .commandBufferInfoCount = 1,
        .pCommandBufferInfos = &cmdSubmitInfo,
        .signalSemaphoreInfoCount = static_cast<u32>(semaphoreSignals.size()),
        .pSignalSemaphoreInfos = semaphoreSignals.data()
    };

    vkQueueSubmit2(_vulkan_device->get_queue(), 1, &submitInfo, VK_NULL_HANDLE);

    auto swapchain = _swapchain->get_swapchain();
    VkPresentInfoKHR presentInfo
    {
        .sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR,
        .waitSemaphoreCount = 1,
        .pWaitSemaphores = &_swapchain->get_render_complete_semaphores()[imageIndex],
        .swapchainCount = 1,
        .pSwapchains = &swapchain,
        .pImageIndices = &imageIndex,
        .pResults = nullptr
    };

    vkQueuePresentKHR(_vulkan_device->get_queue(), &presentInfo);
}
