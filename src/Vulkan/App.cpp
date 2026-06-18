#include "Vulkan/App.hpp"
#include "Core/LoadingScreen.hpp"
#include "Core/WorldScreen.hpp"

#include <cassert>
#include <chrono>

#include "NsRender/VKFactory.h"

namespace lve {

    App::App() {

        resourceManager = std::make_unique<ResourceManager>(device);
        renderer = std::make_unique<Renderer>(device, window.getExtent());

        descriptorManager_ = std::make_unique<DescriptorManager>(device);

        keybinds_ = std::make_unique<KeyBindHandler>();
        keybinds_->setWindow(window.getGLFWWindow());

        keybinds_->onPress({Key::F11}, [this]() {
            window.toggleFullscreen();
            requestSwapchainRecreate = true;
        });

        keybinds_->onPress({Key::ESCAPE}, [this]() {
            glfwSetWindowShouldClose(window.getGLFWWindow(), GLFW_TRUE);
        });

        uiSystem = std::make_unique<UiSystem>();

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

        uiSystem->registerButtonHandler("QuitButton", [this]() {
            glfwSetWindowShouldClose(window.getGLFWWindow(), GLFW_TRUE);
        });

        uiSystem->registerButtonHandler("EnterWorldButton", [this]() {
            screenManager->switchTo<WorldScreen>(device, *resourceManager, *keybinds_, window, renderer->getExtent(), renderer->getSwapChainImageViews(), renderer->getSwapChainImageFormat());
        });

        QueueFamilyIndices indices = device.findPhysicalQueueFamilies();

        NoesisApp::VKFactory::InstanceInfo info{};
        info.instance = device.getInstance();
        info.physicalDevice = device.getPhysicalDevice();
        info.device = device.device();
        info.pipelineCache = VK_NULL_HANDLE;
        info.queueFamilyIndex = indices.graphicsFamily;
        info.vkGetInstanceProcAddr = glfwGetInstanceProcAddress;
        info.stereoSupport = false;

        uiSystem->init(window.getExtent().width, window.getExtent().height, info, renderer->getRenderPass());

        screenManager = std::make_unique<ScreenManager>();
        screenManager->switchTo<LoadingScreen>(device, *descriptorManager_, *resourceManager,
                                                renderer->getRenderPass(), *uiSystem, keybinds_.get());
    }

    App::~App() {
        vkDeviceWaitIdle(device.device());
    }

    bool App::windowShouldClose() const {
        return window.shouldClose();
    }

    void App::tick() {
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
            double currentTime = glfwGetTime();
            double dt = currentTime - prevTime_;
            prevTime_ = currentTime;

            if (dt > 0.25) dt = 0.25;

            tickAccumulator_ += dt;
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
        screenManager->notifySwapChainRecreated(extent, renderer->getSwapChainImageViews(), renderer->getSwapChainImageFormat());
    }


    void App::drawFrame() {
        if (renderState != RenderState::Running)
            return;

        if (!renderer->beginFrame()) {
            recreateSwapChain();
            return;
        }

        double currentTime = glfwGetTime();

        VkCommandBuffer cmd = renderer->getActiveCommandBuffer();
        auto extent = renderer->getExtent();

        if (!dynamic_cast<WorldScreen*>(screenManager->getCurrent())) {
            uiSystem->update(currentTime);
            uiSystem->renderOffscreen(cmd);
        }

        FrameContext frameCtx{};
        frameCtx.cmd = cmd;
        frameCtx.renderPass = renderer->getRenderPass();
        frameCtx.extent = extent;
        frameCtx.dt = currentTime;
        frameCtx.frameIndex = renderer->getFrameIndex();
        frameCtx.imageIndex = renderer->getCurrentImageIndex();

        if (auto* ws = dynamic_cast<WorldScreen*>(screenManager->getCurrent())) {
            RenderPassBegin worldPass{};
            worldPass.renderPass = ws->getWorldRenderPass();
            worldPass.framebuffer = ws->getFramebuffer(frameCtx.imageIndex);
            worldPass.renderArea = {{0, 0}, extent};
            worldPass.viewport = {0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f};
            worldPass.scissor = {{0, 0}, extent};
            worldPass.clearValues = {
                {{0.4f, 0.6f, 0.9f, 1.0f}},
                {1.0f, 0}
            };

            renderer->executeRenderPass(worldPass, [this, frameCtx](VkCommandBuffer cb) {
                screenManager->render(frameCtx);
            });
        } else {
            RenderPassBegin mainPass{};
            mainPass.renderPass = frameCtx.renderPass;
            mainPass.framebuffer = renderer->getCurrentFramebuffer();
            mainPass.renderArea = {{0, 0}, extent};
            mainPass.viewport = {0.0f, 0.0f, static_cast<float>(extent.width), static_cast<float>(extent.height), 0.0f, 1.0f};
            mainPass.scissor = {{0, 0}, extent};
            mainPass.clearValues = {
                {{0.1f, 0.1f, 0.1f, 1.0f}}
            };

            renderer->executeRenderPass(mainPass, [this, frameCtx](VkCommandBuffer cb) {
                screenManager->render(frameCtx);
                uiSystem->render(cb, frameCtx.renderPass);
            });
        }

        if (!renderer->endFrame()) {
            window.resetWindowResizedFlag();
            recreateSwapChain();
        }
    }

}  // namespace lve
