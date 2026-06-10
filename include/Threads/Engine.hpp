#pragma once

#include <memory>

#include "Renderer.hpp"

namespace lve {

    class Engine {
    public:
        static void Init() {
            if (!instance) {
                instance = std::make_unique<Engine>();

            }
        }
        static void Shutdown() {
            instance->stop();
            instance.reset();
        }
        static Engine& Get() {
            return *instance;
        }

        Engine();

        void run();
        void stop();

    private:
        std::thread renderer;
        static std::unique_ptr<Engine> instance;
        std::atomic<bool> running = true;
        std::shared_ptr<Mailbox> mailbox;
    };

}  // namespace lve
