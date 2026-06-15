#pragma once

#include "Vulkan/Device.hpp"
#include "Vulkan/Texture.hpp"
#include <memory>
#include <string>
#include <unordered_map>
#include <functional>
#include <vector>

namespace lve {

class ResourceManager {
public:
    explicit ResourceManager(Device& device);
    ~ResourceManager() = default;

    ResourceManager(const ResourceManager&) = delete;
    ResourceManager& operator=(const ResourceManager&) = delete;

    void loadTexture(const std::string& path, std::function<void(Texture*)> callback);
    void loadShader(const std::string& path, std::function<void(const std::vector<char>&)> callback);

    void garbageCollect();

private:
    Device& device_;
    std::unordered_map<std::string, std::unique_ptr<Texture>> textures_;
    std::unordered_map<std::string, std::vector<char>> shaders_;
};

} // namespace lve
