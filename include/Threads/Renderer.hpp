#pragma once
#include <functional>
#include <memory>
#include "Bus/Mailbox.hpp"

namespace lve {

    class RenderThread {
    public:
        void setQuitCallback(std::function<void()> cb) { quitCallback_ = std::move(cb); }
        void run();

    private:
        std::function<void()> quitCallback_;
        std::shared_ptr<Mailbox> mailbox_;
    };

} // namespace lve
