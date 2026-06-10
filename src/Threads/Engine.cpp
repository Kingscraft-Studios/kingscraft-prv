#include "Threads/Engine.hpp"
#include <chrono>
#include <iostream>

#include "Threads/IO.hpp"

namespace lve {

    std::unique_ptr<Engine> Engine::instance = nullptr;

    Engine::Engine() {
        mailbox = std::make_shared<Mailbox>();
        Renderer::Init();
    }

    void Engine::run() {


        renderer = std::thread([]() {
            Renderer::Get().run();
        });


        while (running.load()) {
            std::this_thread::sleep_for(std::chrono::seconds(1));
        }


    }

    void Engine::stop() {
        running.store(false);
        mailbox->stop();

        Renderer::Shutdown();
        if (renderer.joinable()) renderer.join();
    }

}  // namespace lve
