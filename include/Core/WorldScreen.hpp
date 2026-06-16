#pragma once

#include "Core/Screen.hpp"
#include "Vulkan/Device.hpp"

namespace lve {

    class WorldScreen : public Screen {
    public:
        explicit WorldScreen(Device& device);
        ~WorldScreen() override;

        void init() override;
        void tick(double dt) override;
        void render(const FrameContext& ctx) override;
        void cleanup() override;
        void onRenderPassChanged(VkRenderPass renderPass) override;

    private:
        Device& device_;
    };

} // namespace lve
