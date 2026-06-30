#pragma once
#include <memory>
#include <thread>
#include <string>
#include <vector>


namespace lve {
    class IO {
    public:

        static void Init();
        static void Shutdown();
        static IO& Get();

        void writeLogFile(const std::string& path, const std::string& text);
        void writeFile(const std::string& path, const std::vector<char>& data);
        std::vector<char> readFile(const std::string& path);

    private:

        static std::unique_ptr<IO> instance_;
    };
} // namespace lve
