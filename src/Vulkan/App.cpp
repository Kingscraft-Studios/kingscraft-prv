#include "Vulkan/App.hpp"
#include "Core/MainMenu.hpp"
#include "Core/World/WorldScreen.hpp"
#include "Renderer/Bloom.hpp"
#include "Renderer/RendererSettings.hpp"
#include <chrono>
#include <thread>

namespace lve {

    App* App::instance_ = nullptr;

    App::App() {
        instance_ = this;
        keybinds_->setWindow(window.getGLFWWindow());

        keybinds_->onPress({Key::F11}, [this]() {
            window.toggleFullscreen();
            requestSwapchainRecreate = true;
        });

        keybinds_->onPress({Key::ESCAPE}, [this]() {
            window.setWindowClose();
        });

        window.setMouseMoveCallback([this](double x, double y) {
            uiSystem->onMouseMove(x, y);
        });

        window.setMouseButtonCallback([this](int button, int action, int mods) {
            double x = window.getLastX();
            double y = window.getLastY();

            uiSystem->onMouseButton(button, action, mods, x, y);
        });

        window.setScrollCallback([this](double dx, double dy) {
            uiSystem->onScroll(dx, dy);
        });

        window.setKeyCallback([this](int key, int scancode, int action, int mods) {
            keybinds_->onKeyEvent(key, scancode, action, mods);
        });

        window.setCharCallback([this](unsigned int codepoint) {
            keybinds_->onChar(codepoint);
        });

        keybinds_->setNoesisKeyCallback([this](int key, int action) {
            uiSystem->onKey(key, action);
        });

        keybinds_->setNoesisCharCallback([this](unsigned int codepoint) {
            uiSystem->onChar(codepoint);
        });

        uiSystem->registerButtonHandler(BTN_QUIT_GAME, [this]() {
            window.setWindowClose();
        });

        uiSystem->registerButtonHandler(BTN_ENTER_WORLD, [this]() {
            screenManager->switchTo<WorldScreen>(renderer->getExtent());
        });

        resourceManager->loadRawImageData("resources/textures/logo/Kingscraft-Logo.png",
            [this](unsigned char* pixels, int width, int height) {
                window.setIcon(pixels, width, height);
            });

        // Init new UI engine alongside Noesis
        uiSystem->init();

        screenManager->switchTo<MainMenu>(renderer->getRenderPass(), *uiSystem);

        VkExtent2D extent = window.getExtent();
        postProcessor_ = std::make_unique<PostProcessing>();
        auto bloom = std::make_unique<Bloom>(device, extent, renderer->getWorldRenderPass(), *descriptorManager_);
        postProcessor_->addEffect(std::move(bloom));
    }

    App::~App() {
        vkDeviceWaitIdle(device.device());
    }

    bool App::windowShouldClose() const {
        return window.shouldClose();
    }

    void App::tick() {
        currentTime = TimeUtil::uptimeSeconds();
        dt_ = currentTime - prevTime_;
        prevTime_ = currentTime;
        if (dt_ > 0.25) dt_ = 0.25;

        glfwPollEvents();
        keybinds_->update();
        window.processInput();

        auto currentExtent = window.getExtent();

        if ((requestSwapchainRecreate || window.wasWindowResized()) && currentExtent.width > 0 && currentExtent.height > 0) {

            requestSwapchainRecreate = false;
            pauseRenderer();
            window.resetWindowResizedFlag();

            recreateSwapChain();
            uiSystem->resize(currentExtent.width, currentExtent.height);
            resumeRenderer();
        }

        if (renderState == RenderState::Running) {
            tickAccumulator_ += dt_;
            while (tickAccumulator_ >= TICK_INTERVAL) {
                screenManager->tick(TICK_INTERVAL);
                tickAccumulator_ -= TICK_INTERVAL;
            }

            drawFrame();

            int maxFps = RendererSettings::get().maxFps;
            if (maxFps > 0) {
                double frameTime = TimeUtil::uptimeSeconds() - currentTime;
                double target = 1.0 / maxFps;
                if (frameTime < target) {
                    std::this_thread::sleep_for(std::chrono::duration<double>(target - frameTime));
                }
            }
        }
    }

    void App::cleanup() {
        vkDeviceWaitIdle(device.device());
    }

    void App::pauseRenderer() {
        renderState = RenderState::Paused;

        vkDeviceWaitIdle(device.device());
    }

    void App::resumeRenderer() {
        renderState = RenderState::Running;
    }

    void App::recreateSwapChain() {
        auto extent = window.getExtent();
        while (extent.width == 0 || extent.height == 0) {
            extent = window.getExtent();
            glfwWaitEvents();
        }

        vkDeviceWaitIdle(device.device());

        renderer->recreateSwapChain(extent);
        screenManager->notifyRenderPassChanged(renderer->getRenderPass());
        screenManager->notifySwapChainRecreated(extent);
        auto* bloom = static_cast<Bloom*>(postProcessor_->getEffect("bloom"));
        if (bloom) bloom->recreate(extent, renderer->getWorldRenderPass());
    }


    void App::drawFrame() {
        if (renderState != RenderState::Running) {
            return;
        }

        if (!renderer->beginFrame()) {
            recreateSwapChain();
            return;
        }
        VkCommandBuffer cmd = renderer->getActiveCommandBuffer();
        VkExtent2D extent = renderer->getExtent();
        uint32_t imageIndex = renderer->getCurrentImageIndex();

        auto info = screenManager->getCurrent()->getFrameRenderInfo(*renderer, imageIndex);

        if (info.uiEnabled) {
            uiSystem->update(dt_);
            uiSystem->renderOffscreen(cmd);
        }

        FrameContext frameCtx{};
        frameCtx.cmd = cmd;
        frameCtx.renderPass = info.renderPass;
        frameCtx.extent = extent;
        frameCtx.dt = dt_;
        frameCtx.frameIndex = renderer->getFrameIndex();
        frameCtx.imageIndex = imageIndex;
        frameCtx.postProcessing = postProcessor_.get();

        // Pre-scene effects (glow passes, downsampling, etc.)
        if (!info.uiEnabled) {
            postProcessor_->preScene(frameCtx, [this](const FrameContext& ctx) {
                screenManager->getCurrent()->renderGlow(ctx);
            });
        }

        RenderPassBegin pass{};
        pass.renderPass = info.renderPass;
        pass.framebuffer = info.framebuffer;
        pass.renderArea = {{0, 0}, extent};
        pass.viewport = {0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f};
        pass.scissor = {{0, 0}, extent};
        pass.clearValues = info.clearValues;

        renderer->executeRenderPass(pass, [this, frameCtx, info](VkCommandBuffer cb) {
            screenManager->render(frameCtx);

            // Post-scene effects (composite, etc.)
            if (!info.uiEnabled) {
                postProcessor_->postScene(frameCtx);
            }
        });

        if (!renderer->endFrame()) {
            window.resetWindowResizedFlag();
            recreateSwapChain();
        }
    }

}  // namespace lve
