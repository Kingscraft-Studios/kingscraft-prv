#include "Vulkan/App.hpp"
#include "Core/LoadingScreen.hpp"

#include <cassert>
#include <chrono>

#include "NsRender/VKFactory.h"

namespace lve {

    App::App() {

        window.setFullscreenToggleCallback([this]() {
            requestSwapchainRecreate = true;
        });

        resourceManager = std::make_unique<ResourceManager>(device);
        renderer = std::make_unique<Renderer>(device, window.getExtent());

        descriptorManager_ = std::make_unique<DescriptorManager>(device);

        uiSystem = std::make_unique<UiSystem>();

        window.setMouseMoveCallback([this](double x, double y) {
            uiSystem->onMouseMove(x, y);
        });

        window.setMouseButtonCallback([this](int button, int action, int mods) {
            double x = window.getLastX();
            double y = window.getLastY();

            uiSystem->onMouseButton(button, action, mods, x, y);
        });

        uiSystem->setQuitCallback([this]() {
            glfwSetWindowShouldClose(window.getGLFWWindow(), GLFW_TRUE);
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
                                                renderer->getRenderPass(), *uiSystem);
    }

    App::~App() {
        vkDeviceWaitIdle(device.device());
    }

    bool App::windowShouldClose() const {
        return window.shouldClose();
    }

    void App::tick() {
        glfwPollEvents();
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
            screenManager->tick(currentExtent.width > 0 && currentExtent.height > 0 ? glfwGetTime() : 0.0);
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
    }


    void App::drawFrame() {
        if (renderState != RenderState::Running)
            return;

        if (!renderer->beginFrame()) {
            recreateSwapChain();
            return;
        }

        double currentTime = glfwGetTime();
        uiSystem->update(currentTime);

        VkCommandBuffer cmd = renderer->getActiveCommandBuffer();
        auto extent = renderer->getExtent();

        uiSystem->renderOffscreen(cmd);

        FrameContext frameCtx{};
        frameCtx.cmd = cmd;
        frameCtx.renderPass = renderer->getRenderPass();
        frameCtx.extent = extent;
        frameCtx.dt = currentTime;
        frameCtx.frameIndex = renderer->getFrameIndex();

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

        if (!renderer->endFrame()) {
            window.resetWindowResizedFlag();
            recreateSwapChain();
        }
    }

}  // namespace lve
