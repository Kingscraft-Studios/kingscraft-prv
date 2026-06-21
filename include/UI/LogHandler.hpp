#pragma once
#include "Bus/MessageBus.hpp"
#include "Threads/Logger.hpp"

namespace lve {
    struct NoesisLogHandler {
        static void handler(const char*, uint32_t, uint32_t level, const char* channel, const char* message) {
            LogLevel levelStr;

            if (level == 0) levelStr = LogLevel::ERROR;
            else if (level == 1) levelStr = LogLevel::DEBUG;
            else if (level == 2) levelStr = LogLevel::INFO;
            else if (level == 3) levelStr = LogLevel::WARN;
            else if (level == 4) levelStr = LogLevel::ERROR;

            std::string msgCopy = message ? message : "";

            MessageBus::Get().send(ThreadName::Engine, [levelStr, msgCopy]() {
                Logger::Get().log(levelStr, ThreadName::Renderer, msgCopy);
            });
        }
    };
}
