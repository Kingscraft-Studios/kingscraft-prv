#pragma once

#include "Vulkan/Window.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/RenderPass.hpp"
#include "Vulkan/DescriptorManager.hpp"
#include "UI/UiSystem.hpp"
#include "Renderer/Renderer.hpp"
#include "Resource/ResourceManager.hpp"
#include "Core/ScreenManager.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/KeyBindHandler.hpp"
#include <memory>


namespace lve {

    class App {
    public:
        static constexpr int WIDTH = 900;
        static constexpr int HEIGHT = 650;

        App();
        ~App();
        App(const App &) = delete;
        App &operator=(const App &) = delete;

        bool windowShouldClose() const;
        void tick();
        void cleanup();

        enum class RenderState {
            Running,
            Paused,
            Rebuilding
        };

        void pauseRenderer();
        void resumeRenderer();

    private:
        void drawFrame();
        void recreateSwapChain();

        Window window{WIDTH, HEIGHT, "Kingscraft"};
        Device device{window};
        std::unique_ptr<Renderer> renderer;
        std::unique_ptr<DescriptorManager> descriptorManager_;
        std::unique_ptr<ResourceManager> resourceManager;
        std::unique_ptr<UiSystem> uiSystem;
        std::unique_ptr<ScreenManager> screenManager;
        std::unique_ptr<KeyBindHandler> keybinds_;
        bool requestSwapchainRecreate = false;
        VkExtent2D lastExtent{0, 0};
        RenderState renderState = RenderState::Running;
    };

}  // namespace lve
