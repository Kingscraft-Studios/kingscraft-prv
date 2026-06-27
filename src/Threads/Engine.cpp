#include "Threads/Engine.hpp"
#include "Threads/Logger.hpp"
#include "Threads/IO.hpp"
#include "Bus/MessageBus.hpp"
#include "Util/TimeUtil.hpp"

#include <chrono>

#include "Core/Registries.hpp"
#include "Util/LogUtils.hpp"

namespace lve {

    std::unique_ptr<Engine> Engine::instance_ = nullptr;

    Engine::~Engine() {
        mailbox_->stop();
        resourceLoader_.stop();
        if (resLoaderThread_.joinable()) resLoaderThread_.join();
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

        LogUtils::info(ThreadName::Renderer, "Infrastructure started");

        mailbox_ = std::make_shared<Mailbox>();
        MessageBus::Get().subscribe(ThreadName::Engine, mailbox_);

        LogUtils::info(ThreadName::Renderer, "Mailbox subscribed");

        resLoaderThread_ = std::thread([this]() { resourceLoader_.run(); });

        regThread_ = std::thread([]() { Registries::build(); });
        regThread_.detach();

        renderer_.setQuitCallback([this]() { stop(); });
        rendererThread_ = std::thread([this]() { renderer_.run(); });

        LogUtils::info(ThreadName::Renderer, "Entering game loop");
        while (running_) {
            Message msg;
            while (mailbox_->pop_for(msg, std::chrono::milliseconds(100))) {
                if (msg.payload) msg.payload();
            }
        }

        LogUtils::info(ThreadName::Renderer, "Stopping renderer");
        if (rendererThread_.joinable()) rendererThread_.join();

        resourceLoader_.stop();
        if (resLoaderThread_.joinable()) resLoaderThread_.join();

        MessageBus::Get().unsubscribe(ThreadName::Engine);
        LogUtils::info(ThreadName::Renderer, "Shutting down");

        Logger::Shutdown();
        IO::Shutdown();

        MessageBus::Get().signalQuit();
    }

    void Engine::stop() {
        running_ = false;
        mailbox_->stop();
    }

} // namespace lve
