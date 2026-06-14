#pragma once
#include <memory>
#include <thread>
#include <string>

#include "Bus/Mailbox.hpp"

namespace lve {
    class IO {
    public:

        static void Init();
        static void Shutdown();
        static IO& Get();

        void writeLogFile(const std::string& path, const std::string& text);

    private:

        static std::unique_ptr<IO> instance_;
    };
} // namespace lve
