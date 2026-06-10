#include "Threads/Logger.hpp"

#include <iostream>


namespace lve {
    std::unique_ptr<Logger> Logger::instance = nullptr;

    Logger::Logger() {
        mailbox = std::make_shared<Mailbox>();
    }

    void Logger::start() {
        Message msg;
        while (mailbox->pop(msg)) {
            if (!msg.payload) {
                log("[ERROR] Payload Empty");
                continue;
            }

            msg.payload();
        }
    }

    void Logger::stop() {
        mailbox->stop();
    }


    void Logger::log(std::string data) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            std::cout << data << std::endl;
        }
    }
}
