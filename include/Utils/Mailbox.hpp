#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include "Message.hpp"

class Mailbox {
public:
    void push(Message msg) {
        {
            std::lock_guard<std::mutex> lock(mtx);
            queue.push(std::move(msg));
        }
        cv.notify_one();
    }

    bool pop(Message& out) {
        std::unique_lock<std::mutex> lock(mtx);

        cv.wait(lock, [&]() {
            return !queue.empty() || stopped;
        });

        if (queue.empty())
            return false;

        out = std::move(queue.front());
        queue.pop();
        return true;
    }

    void stop() {
        {
            std::lock_guard<std::mutex> lock(mtx);
            stopped = true;
        }
        cv.notify_all();
    }

private:
    std::queue<Message> queue;
    std::mutex mtx;
    std::condition_variable cv;
    bool stopped = false;
};