#pragma once

#include <NoesisPCH.h>
#include <vulkan/vulkan.h>
#include <NsRender/VKFactory.h>
#include "Core/Constants.hpp"

namespace lve {
    class UiSystem {
    public:
        ~UiSystem();

        void init(int width, int height, const NoesisApp::VKFactory::InstanceInfo& vkInfo, VkRenderPass renderPass);
        void shutdown();
        void update(double time);
        void resize(int width, int height);
        void onMouseMove(double x, double y);
        void onMouseButton(int button, int action, int mods, double x, double y);

        void renderOffscreen(VkCommandBuffer cmdBuffer);
        void render(VkCommandBuffer cmdBuffer, VkRenderPass renderPass);
        void setQuitCallback(std::function<void()> cb) { quitCallback = cb; }
        void loadXaml(const std::string& xamlPath);
        Noesis::IView* getView() const { return view; }
    private:
        bool initialized = false;
        Noesis::Ptr<Noesis::IView> view;
        Noesis::Ptr<Noesis::RenderDevice> device;
        uint64_t frame = 0;
        uint64_t safeFrame;
        int width_ = 0;
        int height_ = 0;
        std::function<void()> quitCallback;
    };
}
