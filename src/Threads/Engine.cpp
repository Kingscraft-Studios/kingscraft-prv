#include "Threads/Engine.hpp"
#include "Threads/Logger.hpp"
#include "Threads/IO.hpp"
#include "Bus/MessageBus.hpp"
#include "Util/TimeUtil.hpp"

#include <chrono>

namespace lve {

    std::unique_ptr<Engine> Engine::instance_ = nullptr;

    Engine::~Engine() {
        mailbox_->stop();
        if (rendererThread_.joinable()) rendererThread_.join();
    }

    void Engine::Init() {
        instance_ = std::make_unique<Engine>();
    }

    void Engine::Shutdown() {
        instance_.reset();
    }

    Engine& Engine::Get() {
        return *instance_;
    }

    void Engine::run() {
        TimeUtil::Init();
        IO::Init();
        Logger::Init();

        Logger::Get().log(LogLevel::INFO, ThreadName::Engine, "Infrastructure started");

        mailbox_ = std::make_shared<Mailbox>();
        MessageBus::Get().subscribe(ThreadName::Engine, mailbox_);

        Logger::Get().log(LogLevel::INFO, ThreadName::Engine, "Mailbox subscribed");

        renderer_.setQuitCallback([this]() { stop(); });
        rendererThread_ = std::thread([this]() { renderer_.run(); });

        Logger::Get().log(LogLevel::INFO, ThreadName::Engine, "Entering game loop");
        while (running_) {
            Message msg;
            while (mailbox_->pop_for(msg, std::chrono::milliseconds(100))) {
                if (msg.payload) msg.payload();
            }
        }

        Logger::Get().log(LogLevel::INFO, ThreadName::Engine, "Stopping renderer");
        if (rendererThread_.joinable()) rendererThread_.join();

        MessageBus::Get().unsubscribe(ThreadName::Engine);
        Logger::Get().log(LogLevel::INFO, ThreadName::Engine, "Shutting down");

        Logger::Shutdown();
        IO::Shutdown();

        MessageBus::Get().signalQuit();
    }

    void Engine::stop() {
        running_ = false;
        mailbox_->stop();
    }

} // namespace lve
