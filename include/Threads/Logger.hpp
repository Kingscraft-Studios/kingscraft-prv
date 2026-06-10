#pragma once
#include <memory>

#include "Utils/Mailbox.hpp"

namespace lve {
    class Logger {
    public:
        static void Init() {
            if (!instance) {
                instance = std::make_unique<Logger>();
            }
        }
        static Logger& Get() {
            return *instance;
        }
        static void Shutdown() {
            instance->stop();
            instance.reset();
        }

        Logger();

        void start();
        void stop();

        void log(std::string data);

    private:
        static std::unique_ptr<Logger> instance;
        std::shared_ptr<Mailbox> mailbox;

        std::mutex mtx;
    };
}
