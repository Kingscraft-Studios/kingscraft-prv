#include "Threads/IO.hpp"

#include <fstream>
#include <iostream>
#include <mutex>


namespace lve {
    std::unique_ptr<IO> IO::instance = nullptr;

    IO::IO() {
        running = true;
        worker = std::thread(&IO::run, this);
        mailbox = std::make_shared<Mailbox>();
    }

    void IO::start() {
        Message msg;

        while (mailbox->pop(msg)) {
            if (!msg.payload) {
                std::cout << "[ERROR] Payload Empty\n";
                continue;
            }

            {
                std::lock_guard<std::mutex> lock(mtx);
                queue.push(msg.payload); // 🔥 forward to worker
            }

            cv.notify_one();
        }
    }

    void IO::run() {
        while (true) {
            std::function<void()> job;

            {
                std::unique_lock<std::mutex> lock(mtx);

                cv.wait(lock, [&] {
                    return !queue.empty() || !running;
                });

                if (!running && queue.empty())
                    break;

                job = std::move(queue.front());
                queue.pop();
            }

            job(); // 🔥 Execute the task
        }
    }

    void IO::write(const std::string& path, const std::string& text, bool append) {
        {
            std::lock_guard<std::mutex> lock(mtx);

            queue.push([path, text, append]() {
                std::ofstream file;

                if (append)
                    file.open(path, std::ios::app);
                else
                    file.open(path, std::ios::trunc);

                if (file.is_open()) {
                    file << text;
                    file.flush();
                }
            });
        }
        cv.notify_one();
    }

    void IO::stop() {
        mailbox->stop();
        {
            std::lock_guard<std::mutex> lock(mtx);
            running = false;
        }
        cv.notify_all();

        if (worker.joinable())
            worker.join();

    }
}
