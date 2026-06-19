#include <iostream>
#include <memory>

#include <GLFW/glfw3.h>
#include "Core/KeyCodes.hpp"
#include "NsApp/LocalFontProvider.h"
#include "NsApp/LocalXamlProvider.h"
#include "Threads/Logger.hpp"
#include "UI/LogHandler.hpp"
#include "UI/UiSystem.hpp"

namespace lve {
    UiSystem::~UiSystem() {
        shutdown();
    }

    void UiSystem::init(int width, int height, const NoesisApp::VKFactory::InstanceInfo& vkinfo,
                        VkRenderPass renderPass) {
        width_ = width;
        height_ = height;
        Noesis::GUI::SetLogHandler(NoesisLogHandler::handler);
        Noesis::GUI::Init();
        initialized = true;

        device = NoesisApp::VKFactory::CreateDevice(true, vkinfo);

        // TODO: use cache .bin instead of raw .xaml! Validation steps: Store hash of original XAML inside .bin and try parse bin as 2 safety nets
        // TODO: For Vulkan Do not forget to wrap the cache in a header with at least a Hash, Version and Magic
        Noesis::GUI::SetXamlProvider(Noesis::MakePtr<NoesisApp::LocalXamlProvider>("resources/ui"));
        Noesis::GUI::SetFontProvider(Noesis::MakePtr<NoesisApp::LocalFontProvider>("resources/fonts"));

        loadXaml("MainMenu.xaml");
    }

    void UiSystem::loadXaml(const std::string& xamlPath) {
        if (!initialized || !device) return;

        if (view) {
            view->GetRenderer()->Shutdown();
            view.Reset();
        }

        auto xaml = Noesis::GUI::LoadXaml<Noesis::FrameworkElement>(xamlPath.c_str());
        view = Noesis::GUI::CreateView(xaml);
        view->Activate();
        view->SetFlags(Noesis::RenderFlags_PPAA | Noesis::RenderFlags_LCD);
        wireButtons(xaml);
        view->SetSize(width_, height_);
        view->GetRenderer()->Init(device);
    }

    void UiSystem::shutdown() {
        if (!initialized) return;

        if (view) {
            view->GetRenderer()->Shutdown();
            view.Reset();
            device.Reset();
        }

        // 2. Shut down the engine
        Noesis::GUI::Shutdown();

        initialized = false;
    }

    void UiSystem::registerButtonHandler(std::string name, std::function<void()> handler) {
        buttonHandlers_[std::move(name)] = std::move(handler);
    }

    void UiSystem::wireButtons(Noesis::FrameworkElement* xaml) {
        for (auto& [name, handler] : buttonHandlers_) {
            Noesis::Button* btn = xaml->FindName<Noesis::Button>(name.c_str());
            if (btn) {
                auto shared = std::make_shared<std::function<void()>>(handler);
                btn->Click() += [shared](Noesis::BaseComponent*, const Noesis::RoutedEventArgs&) {
                    if (*shared) (*shared)();
                };
            }
        }
    }

    void UiSystem::onMouseMove(double x, double y) {
        if (!view) return;

        view->MouseMove((float)x, (float)y);
    }

    void UiSystem::onMouseButton(int button, int action, int mods, double x, double y) {
        if (!view) return;

        Noesis::MouseButton nButton;
        if (button == GLFW_MOUSE_BUTTON_LEFT) nButton = Noesis::MouseButton_Left;
        else if (button == GLFW_MOUSE_BUTTON_RIGHT) nButton = Noesis::MouseButton_Right;
        else return;

        if (action == GLFW_PRESS) {
            view->MouseButtonDown((float)x, (float)y, nButton);
        }
        else if (action == GLFW_RELEASE) {
            view->MouseButtonUp((float)x, (float)y, nButton);
        }
    }

    void UiSystem::onKey(int key, int action) {
        if (!view) return;
        Noesis::Key nk = glfwKeyToNoesis(key);
        if (nk == Noesis::Key_None) return;
        if (action == GLFW_PRESS || action == GLFW_REPEAT)
            view->KeyDown(nk);
        else if (action == GLFW_RELEASE)
            view->KeyUp(nk);
    }

    void UiSystem::onChar(unsigned int codepoint) {
        if (!view) return;
        view->Char(codepoint);
    }

    void UiSystem::update(double time) {
        if (view) {
            view->Update(time);
        }
    }

    void UiSystem::resize(int width, int height) {
        if (view) {
            view->SetSize(width, height);
        }
    }

    void UiSystem::renderOffscreen(VkCommandBuffer cmdBuffer) {
        if (!view || !device) return;

        // Update safeFrame dynamically every frame
        if (frame >= lve::MAX_FRAMES_IN_FLIGHT) {
            safeFrame = frame - lve::MAX_FRAMES_IN_FLIGHT;
        }
        else {
            safeFrame = 0;
        }

        // Set the command buffer for the GPU work
        NoesisApp::VKFactory::SetCommandBuffer(device, {cmdBuffer, frame, safeFrame});

        auto renderer = view->GetRenderer();
        renderer->UpdateRenderTree();

        // THIS MUST BE CALLED OUTSIDE vkCmdBeginRenderPass
        renderer->RenderOffscreen();
    }

    void UiSystem::render(VkCommandBuffer cmdBuffer, VkRenderPass renderPass) {
        if (!view || !device) return;

        // Tell Noesis which RenderPass we are currently inside
        NoesisApp::VKFactory::SetRenderPass(device, renderPass, 1);

        // THIS MUST BE CALLED INSIDE vkCmdBeginRenderPass
        view->GetRenderer()->Render();

        frame++;
    }
}
