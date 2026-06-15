#include "Threads/IO.hpp"

#include <filesystem>

#include <fstream>
#include <stdexcept>

namespace lve {

    std::unique_ptr<IO> IO::instance_ = nullptr;

    void IO::Init() {
        instance_ = std::make_unique<IO>();
    }

    void IO::Shutdown() {
        if (!instance_) return;
        instance_.reset();
    }

    IO& IO::Get() {
        return *instance_;
    }

    std::vector<char> IO::readFile(const std::string& path) {
        std::ifstream file{path, std::ios::ate | std::ios::binary};

        if (!file.is_open()) {
            throw std::runtime_error("failed to open file: " + path);
        }

        size_t fileSize = static_cast<size_t>(file.tellg());
        std::vector<char> buffer(fileSize);

        file.seekg(0);
        file.read(buffer.data(), fileSize);

        file.close();
        return buffer;
    }

    void IO::writeLogFile(const std::string& path, const std::string& text) {

        // 1. ensure directory exists
        std::filesystem::path filePath(path);
        std::filesystem::create_directories(filePath.parent_path());

        // 2. write file
        std::ofstream file(path, std::ios::app);
        if (file.is_open()) {
            file << text << std::endl;
        }
    }

} // namespace lve
