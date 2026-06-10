#include "Threads/Renderer.hpp"
#include "Vulkan/App.hpp"
#include <iostream>

namespace lve {

    std::unique_ptr<Renderer> Renderer::instance = nullptr;

    Renderer::Renderer() {
    }

    void Renderer::run() {
        try {
            App app;
            app.run();
        } catch (std::exception e) {
            std::cout << "Exceptiom: "  << e.what() << std::endl;
        }

    }


    void Renderer::stop() {
    }

} // namespace lve
