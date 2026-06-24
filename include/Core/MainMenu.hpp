#pragma once

#include "Core/Screen.hpp"
#include "UI/UiWrapper.hpp"

namespace lve {

    class MainMenu : public Screen {
    public:
        MainMenu(VkRenderPass renderPass, UiWrapper& uiSystem);

        void init() override {}
        void tick(double) override {}
        void render(const FrameContext& ctx) override { uiSystem_.render(ctx.cmd, ctx.renderPass); }
        void cleanup() override {}
        void onRenderPassChanged(VkRenderPass renderPass) override { renderPass_ = renderPass; }

        FrameRenderInfo getFrameRenderInfo(const Renderer& renderer, uint32_t) const override;

    private:
        VkRenderPass renderPass_;
        UiWrapper& uiSystem_;
    };

} // namespace lve
