#pragma once
#include <memory>
#include <shared_mutex>
#include <unordered_map>
#include <functional>
#include <condition_variable>

#include "Utils/Message.hpp"
#include "Utils/Mailbox.hpp"

class MessageBus {
public:
    static void Init();
    static void Shutdown();
    static MessageBus& Get();

    void subscribe(ThreadName name, std::shared_ptr<Mailbox> mailbox);
    void unsubscribe(ThreadName name);

    bool send(ThreadName target, Message msg);
    bool send(ThreadName target, std::function<void()> payload);

    void signalQuit();
    void waitForQuit();

private:
    MessageBus() = default;

    std::shared_mutex rwMutex_;
    std::unordered_map<ThreadName, std::weak_ptr<Mailbox>> subscribers_;

    std::mutex quitMutex_;
    std::condition_variable quitCV_;
    bool quitting_ = false;
};
