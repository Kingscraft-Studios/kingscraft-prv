#pragma once
#include <functional>
#include "Bus/MessageBus.hpp"

enum class SType {
    Send,
    Request
};

class BusUtil {
public:
    static void structure(SType type, ThreadName target, std::function<void()> payload) {
        if (type == SType::Send) {
            MessageBus::Get().send(target, std::move(payload));
        }
    }

    template<typename T>
    static void structure(SType type, ThreadName target, std::function<T()> work,
                          ThreadName replyTo, std::function<void(T)> callback) {
        if (type == SType::Request) {
            MessageBus::Get().request<T>(target, std::move(work), replyTo, std::move(callback));
        }
    }
};
