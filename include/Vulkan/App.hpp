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
        static constexpr int WIDTH = DEFAULT_WINDOW_WIDTH;
        static constexpr int HEIGHT = DEFAULT_WINDOW_HEIGHT;

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
        std::unique_ptr<Renderer> renderer = std::make_unique<Renderer>(device, window.getExtent());
        std::unique_ptr<DescriptorManager> descriptorManager_ = std::make_unique<DescriptorManager>(device);
        std::unique_ptr<ResourceManager> resourceManager = std::make_unique<ResourceManager>(device);
        std::unique_ptr<UiSystem> uiSystem = std::make_unique<UiSystem>();
        std::unique_ptr<ScreenManager> screenManager = std::make_unique<ScreenManager>();
        std::unique_ptr<KeyBindHandler> keybinds_ = std::make_unique<KeyBindHandler>();
        static constexpr double TICK_RATE = 100.0;
        static constexpr double TICK_INTERVAL = 1.0 / TICK_RATE;

        QueueFamilyIndices indices = device.findPhysicalQueueFamilies();
        bool requestSwapchainRecreate = false;
        VkExtent2D lastExtent{0, 0};
        RenderState renderState = RenderState::Running;
        double prevTime_ = 0.0;
        double tickAccumulator_ = 0.0;
    };

}  // namespace lve
