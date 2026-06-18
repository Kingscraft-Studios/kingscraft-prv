#pragma once

#include "Core/FrameContext.hpp"
#include <vector>

namespace lve {

    class Screen {
    public:
        virtual ~Screen() = default;

        virtual void init() = 0;
        virtual void tick(double dt) = 0;
        virtual void render(const FrameContext& ctx) = 0;
        virtual void cleanup() = 0;

        virtual void onRenderPassChanged(VkRenderPass) {}
        virtual void onSwapChainRecreated(VkExtent2D extent, const std::vector<VkImageView>& swapChainImageViews, VkFormat swapChainImageFormat) {}
    };

} // namespace lve
