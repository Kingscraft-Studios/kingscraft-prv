#pragma once
#include <functional>

enum class ThreadName {
    Engine,
    Renderer
};

struct Message {
    ThreadName target;
    std::function<void()> payload;
};
