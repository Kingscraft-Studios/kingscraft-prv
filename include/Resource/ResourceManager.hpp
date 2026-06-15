#pragma once

#include "Vulkan/Device.hpp"
#include "Vulkan/Texture.hpp"
#include <memory>
#include <string>
#include <unordered_map>

namespace lve {

class ResourceManager {
public:
    explicit ResourceManager(Device& device);
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    Texture* loadTexture(const std::string& path);

    // Future extension points (same pattern):
    // Mesh* loadMesh(const std::string& path);
    // Shader* loadShader(const std::string& path);
    // Material* loadMaterial(const std::string& path);

    void garbageCollect();

private:
    Device& device_;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
};

} // namespace lve
