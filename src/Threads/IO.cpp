#include "Threads/IO.hpp"

#include <filesystem>

#include <fstream>

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
