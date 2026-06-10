#pragma once

#include <NoesisPCH.h>
#include <vulkan/vulkan.h>
#include <NsRender/VKFactory.h>
#include "Vulkan/Swapchain.hpp"

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

        void renderOffscreen(VkCommandBuffer cmdBuffer); // New
        void render(VkCommandBuffer cmdBuffer);      // Renamed from renders
        void setQuitCallback(std::function<void()> cb) { quitCallback = cb; }
    private:
        bool initialized = false;
        Noesis::Ptr<Noesis::IView> view;
        Noesis::Ptr<Noesis::RenderDevice> device;
        VkRenderPass Pass = nullptr;
        uint64_t frame = 0;
        uint64_t safeFrame;
        std::function<void()> quitCallback;
    };
}
