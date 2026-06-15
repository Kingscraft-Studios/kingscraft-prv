#include "Resource/ResourceManager.hpp"
#include "Threads/IO.hpp"
#include "Bus/BusUtil.hpp"

namespace lve {

ResourceManager::ResourceManager(Device& device)
    : device_(device) {
}

void ResourceManager::loadTexture(const std::string& path,
    std::function<void(Texture*)> callback) {
    auto it = textures_.find(path);
    if (it != textures_.end()) {
        if (callback) callback(it->second.get());
        return;
    }

    BusUtil::structure<std::vector<char>>(
        SType::Request,
        ThreadName::Engine,
        [path]() { return IO::Get().readFile(path); },
        ThreadName::Renderer,
        [this, path, callback = std::move(callback)](std::vector<char> fileData) mutable {
            auto texture = std::make_unique<Texture>(device_, fileData);
            auto* ptr = texture.get();
            textures_[path] = std::move(texture);
            if (callback) callback(ptr);
        }
    );
}

void ResourceManager::loadShader(const std::string& path,
    std::function<void(const std::vector<char>&)> callback) {
    auto it = shaders_.find(path);
    if (it != shaders_.end()) {
        if (callback) callback(it->second);
        return;
    }

    BusUtil::structure<std::vector<char>>(
        SType::Request,
        ThreadName::Engine,
        [path]() { return IO::Get().readFile(path); },
        ThreadName::Renderer,
        [this, path, callback = std::move(callback)](std::vector<char> data) mutable {
            auto& cached = shaders_[path] = std::move(data);
            if (callback) callback(cached);
        }
    );
}

void ResourceManager::garbageCollect() {
    // Future: release textures with no external references
}

} // namespace lve
