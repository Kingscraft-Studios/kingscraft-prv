#pragma once

#include <string>
#include <vector>
#include <unordered_map>
#include <memory>

namespace lve {

class Preloader {
public:
    static void Init();
    static void Shutdown();
    static Preloader& Get();

    void loadAll();

    const std::vector<char>& getShader(const std::string& path) const;
    const std::vector<char>& getPipelineCacheData() const;
    bool hasPipelineCacheData() const { return !pipelineCacheData_.empty(); }

private:

    void loadShaderFile(const std::string& path);

    std::unordered_map<std::string, std::vector<char>> shaders_;
    std::vector<char> pipelineCacheData_;

    static std::unique_ptr<Preloader> instance_;
};

} // namespace lve
