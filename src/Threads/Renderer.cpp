#include "Threads/Renderer.hpp"
#include "Vulkan/App.hpp"

namespace lve {

    void Renderer::run() {
        App app;
        app.run();

        if (quitCallback_) quitCallback_();
    }

} // namespace lve
