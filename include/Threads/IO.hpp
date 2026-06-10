#pragma once
#include <condition_variable>
#include <iostream>
#include <memory>
#include <queue>

#include "Utils/Mailbox.hpp"

namespace lve {
    class IO {
    public:
        static void Init() {
            if (!instance) {
                instance = std::make_unique<IO>();
            }
        }

        static void Shutdown() {
            instance->stop();
            instance.reset();
        }

        static IO& Get() {
            return *instance;
        }

        IO();
        void write(const std::string& path, const std::string& text, bool append = true);

        void start();
        void run();
        void stop();

    private:
        std::queue<std::function<void()>> queue;

        static std::unique_ptr<IO> instance;
        std::mutex mtx;
        std::condition_variable cv;
        std::shared_ptr<Mailbox> mailbox;

        std::thread worker;
        bool running = false;
    };
}
