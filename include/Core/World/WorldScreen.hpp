#pragma once

#include "Core/Screen.hpp"
#include "Core/Camera.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/World/Chunk.hpp"
#include "Core/World/World.hpp"
#include "Vulkan/Pipeline.hpp"
#include <memory>
#include <glm/glm.hpp>

namespace lve {

    class WorldScreen : public Screen {
    public:
        explicit WorldScreen(VkExtent2D extent);
        ~WorldScreen() override;

        void init() override;
        void tick(double dt) override;
        void render(const FrameContext& ctx) override;
        void renderGlow(const FrameContext& ctx) override;
        void cleanup() override;
        void onRenderPassChanged(VkRenderPass renderPass) override;
        void onSwapChainRecreated(VkExtent2D extent) override;

        FrameRenderInfo getFrameRenderInfo(const Renderer& renderer, uint32_t imageIndex) const override;

    private:
        void createPipelineLayout();
        void createPipeline();

        VkExtent2D extent_{};

        VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
        std::unique_ptr<Pipeline> pipeline_;
        std::vector<char> vertShaderCode_;
        std::vector<char> fragShaderCode_;

        std::unique_ptr<World> world_;

        Camera camera_;
        double lastMouseX_ = 0.0;
        double lastMouseY_ = 0.0;
        bool cursorCaptured_ = false;
    };

} // namespace lve
