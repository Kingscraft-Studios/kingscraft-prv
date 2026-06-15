#include "Resource/ResourceManager.hpp"

namespace lve {

ResourceManager::ResourceManager(Device& device)
    : device_(device) {
}

Texture* ResourceManager::loadTexture(const std::string& path) {
    auto it = textures_.find(path);
    if (it != textures_.end()) {
        return it->second.get();
    }

    auto texture = std::make_unique<Texture>(device_, path);
    auto* ptr = texture.get();
    textures_[path] = std::move(texture);
    return ptr;
}

void ResourceManager::garbageCollect() {
    // Future: release textures with no external references
}

} // namespace lve
