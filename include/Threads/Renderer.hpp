#pragma once
#include <functional>

namespace lve {

    class Renderer {
    public:
        void setQuitCallback(std::function<void()> cb) { quitCallback_ = std::move(cb); }
        void run();

    private:
        std::function<void()> quitCallback_;
    };

} // namespace lve
