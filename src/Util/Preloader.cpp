#include "Util/Preloader.hpp"
#include "Threads/IO.hpp"

#include <stdexcept>
#include <algorithm>

#include "Util/LogUtils.hpp"

namespace lve {

std::unique_ptr<Preloader> Preloader::instance_ = nullptr;

void Preloader::Init() {
    instance_ = std::make_unique<Preloader>();
}

void Preloader::Shutdown() {
    instance_.reset();
}

Preloader& Preloader::Get() {
    return *instance_;
}

void Preloader::loadAll() {
    // Shaders
    loadShaderFile("resources/shaders/terrain.vert.spv");
    loadShaderFile("resources/shaders/terrain.frag.spv");
    loadShaderFile("resources/shaders/ui.vert.spv");
    loadShaderFile("resources/shaders/ui.frag.spv");
    loadShaderFile("resources/shaders/composite.vert.spv");
    loadShaderFile("resources/shaders/composite.frag.spv");
    loadShaderFile("resources/shaders/PostProcess/bloom/gaussblur.vert.spv");
    loadShaderFile("resources/shaders/PostProcess/bloom/gaussblur.frag.spv");
    loadShaderFile("resources/shaders/PostProcess/bloom/colorpass.vert.spv");
    loadShaderFile("resources/shaders/PostProcess/bloom/colorpass.frag.spv");

    // Pipeline cache
    try {
        pipelineCacheData_ = IO::Get().readFile("pipeline_cache.bin");
    } catch (std::runtime_error e) {
        LogUtils::error(ThreadName::Renderer, StringBuilder::build("Error: ", e.what()));
    }
}

const std::vector<char>& Preloader::getShader(const std::string& path) const {
    auto it = shaders_.find(path);
    if (it == shaders_.end()) {
        throw std::runtime_error("Preloader: shader not preloaded: " + path);
    }
    return it->second;
}

const std::vector<char>& Preloader::getPipelineCacheData() const {
    return pipelineCacheData_;
}

void Preloader::loadShaderFile(const std::string& path) {
    auto data = IO::Get().readFile(path);
    shaders_[path] = std::move(data);
}

} // namespace lve
