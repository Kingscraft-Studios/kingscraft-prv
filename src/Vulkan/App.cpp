#include "Vulkan/App.hpp"
#include "Core/MainMenu.hpp"
#include "Core/WorldScreen.hpp"
#include <chrono>

#include "NsRender/VKFactory.h"

namespace lve {

    App* App::instance_ = nullptr;

    App::App() {
        instance_ = this;
        TimeUtil::setGlfwTime(glfwGetTime());
        keybinds_->setWindow(window.getGLFWWindow());

        keybinds_->onPress({Key::F11}, [this]() {
            window.toggleFullscreen();
            requestSwapchainRecreate = true;
        });

        window.setMouseMoveCallback([this](double x, double y) {
            uiSystem->onMouseMove(x, y);
        });

        window.setMouseButtonCallback([this](int button, int action, int mods) {
            double x = window.getLastX();
            double y = window.getLastY();

            uiSystem->onMouseButton(button, action, mods, x, y);
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

        NoesisApp::VKFactory::InstanceInfo info{};
        info.instance = device.getInstance();
        info.physicalDevice = device.getPhysicalDevice();
        info.device = device.device();
        info.pipelineCache = VK_NULL_HANDLE;
        info.queueFamilyIndex = indices.graphicsFamily;
        info.vkGetInstanceProcAddr = glfwGetInstanceProcAddress;
        info.stereoSupport = false;

        uiSystem->init(window.getExtent().width, window.getExtent().height, info, renderer->getRenderPass());

        screenManager->switchTo<MainMenu>(renderer->getRenderPass(), *uiSystem);
    }

    App::~App() {
        vkDeviceWaitIdle(device.device());
    }

    bool App::windowShouldClose() const {
        return window.shouldClose();
    }

    void App::tick() {
        currentTime = TimeUtil::getGlfwTime();
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
    }


    void App::drawFrame() {
        if (renderState != RenderState::Running)
            return;

        if (!renderer->beginFrame()) {
            recreateSwapChain();
            return;
        }
        VkCommandBuffer cmd = renderer->getActiveCommandBuffer();
        VkExtent2D extent = renderer->getExtent();
        uint32_t imageIndex = renderer->getCurrentImageIndex();

        auto info = screenManager->getCurrent()->getFrameRenderInfo(*renderer, imageIndex);

        if (info.uiEnabled) {
            uiSystem->update(currentTime);
            uiSystem->renderOffscreen(cmd);
        }

        FrameContext frameCtx{};
        frameCtx.cmd = cmd;
        frameCtx.renderPass = info.renderPass;
        frameCtx.extent = extent;
        frameCtx.dt = dt_;
        frameCtx.frameIndex = renderer->getFrameIndex();
        frameCtx.imageIndex = imageIndex;

        RenderPassBegin pass{};
        pass.renderPass = info.renderPass;
        pass.framebuffer = info.framebuffer;
        pass.renderArea = {{0, 0}, extent};
        pass.viewport = {0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f};
        pass.scissor = {{0, 0}, extent};
        pass.clearValues = info.clearValues;

        renderer->executeRenderPass(pass, [this, frameCtx](VkCommandBuffer cb) {
            screenManager->render(frameCtx);
        });

        if (!renderer->endFrame()) {
            window.resetWindowResizedFlag();
            recreateSwapChain();
        }
    }

}  // namespace lve
