#pragma once
#include <functional>

enum class ThreadName {
    Main,
    Render,
    IO,
    Logger
};

struct Message {
    ThreadName sender;
    ThreadName target;
    std::function<void()> payload;
};