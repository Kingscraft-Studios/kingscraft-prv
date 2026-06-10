#pragma once
#include <memory>
#include <thread>
#include <atomic>
#include "Mailbox.hpp"

struct Worker {
    std::shared_ptr<Mailbox> mailbox;
    std::thread thread;
    std::atomic<bool> running = true;
};