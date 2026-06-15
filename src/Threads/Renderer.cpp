#include "Threads/Renderer.hpp"
#include "Vulkan/App.hpp"
#include "Bus/MessageBus.hpp"

namespace lve {

    void RenderThread::run() {
        mailbox_ = std::make_shared<Mailbox>();
        MessageBus::Get().subscribe(ThreadName::Renderer, mailbox_);

        App app;
        while (!app.windowShouldClose()) {
            Message msg;
            while (mailbox_->try_pop(msg)) {
                if (msg.payload) msg.payload();
            }
            app.tick();
        }
        app.cleanup();

        MessageBus::Get().unsubscribe(ThreadName::Renderer);
        if (quitCallback_) quitCallback_();
    }

} // namespace lve
