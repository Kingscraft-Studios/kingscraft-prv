#pragma once

#include <memory>
#include <thread>
#include <atomic>

#include "Bus/Mailbox.hpp"
#include "Renderer.hpp"

namespace lve {

    class Engine {
    public:
        Engine() = default;
        ~Engine();

        static void Init();
        static void Shutdown();
        static Engine& Get();

        void run();
        void stop();

    private:
        std::atomic<bool> running_{true};
        std::shared_ptr<Mailbox> mailbox_;
        std::thread rendererThread_;
        Renderer renderer_;

        static std::unique_ptr<Engine> instance_;
    };

} // namespace lve
