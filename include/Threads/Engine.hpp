#pragma once

#include <memory>
#include <thread>
#include <atomic>

#include "Bus/Mailbox.hpp"
#include "Threads/Renderer.hpp"

namespace lve {

    class Engine {
    public:
        Engine() = default;
        ~Engine();

        static void Init();
        static void Shutdown();
        static Engine& Get();
        static Engine& get() { return Get(); }

        void run();
        void stop();

        Mailbox& getMailbox() { return *mailbox_; }

    private:
        std::atomic<bool> running_{true};
        std::shared_ptr<Mailbox> mailbox_;
        std::thread rendererThread_;
        RenderThread renderer_;

        static std::unique_ptr<Engine> instance_;
    };

} // namespace lve
