#pragma once

#include <memory>

#include "Utils/Mailbox.hpp"

namespace lve {

    class Renderer {
    public:
        static void Init() {
            if (!instance)
                instance = std::make_unique<Renderer>();
        }
        static void Shutdown() {
            instance->stop();
            instance.reset();
        }
        static Renderer& Get() {
            return *instance;
        }

        Renderer();

        void run();
        void stop();

    private:
        static std::unique_ptr<Renderer> instance;

    };

} // namespace lve
