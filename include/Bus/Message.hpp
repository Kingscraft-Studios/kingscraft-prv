#pragma once
#include <functional>

namespace lve {

enum class ThreadName {
    Engine,
    Renderer
};

struct Message {
    ThreadName target;
    std::function<void()> payload;
};

}
