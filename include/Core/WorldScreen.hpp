#pragma once

#include "Core/Screen.hpp"
#include "Core/Camera.hpp"
#include "Core/KeyCodes.hpp"
#include "Core/KeyBindHandler.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/RenderPass.hpp"
#include "Vulkan/Pipeline.hpp"
#include "Vulkan/Window.hpp"
#include "Resource/ResourceManager.hpp"
#include "Core/Chunk.hpp"
#include "Core/ChunkMesher.hpp"
#include "Core/TerrainGenerator.hpp"
#include <vector>
#include <memory>
#include <glm/glm.hpp>

namespace lve {

    class WorldScreen : public Screen {
    public:
        WorldScreen(Device& device, ResourceManager& resourceManager,
                    KeyBindHandler& keybinds, Window& window,
                    VkExtent2D extent, const std::vector<VkImageView>& swapChainImageViews,
                    VkFormat swapChainImageFormat);
        ~WorldScreen() override;

        void init() override;
        void tick(double dt) override;
        void render(const FrameContext& ctx) override;
        void cleanup() override;
        void onRenderPassChanged(VkRenderPass renderPass) override;
        void onSwapChainRecreated(VkExtent2D extent, const std::vector<VkImageView>& swapChainImageViews, VkFormat swapChainImageFormat) override;

        VkRenderPass getWorldRenderPass() const { return worldRenderPass_->getHandle(); }
        VkFramebuffer getFramebuffer(uint32_t imageIndex) const { return framebuffers_[imageIndex]; }

    private:
        VkFormat findDepthFormat();

        void createDepthResources(VkExtent2D extent);
        void createFramebuffers(const std::vector<VkImageView>& swapChainImageViews, VkExtent2D extent);
        void destroyDepthResources();
        void destroyFramebuffers();

        void createPipelineLayout();
        void createPipeline();

        Device& device_;
        ResourceManager& resourceManager_;
        KeyBindHandler& keybinds_;
        Window& window_;
        VkExtent2D extent_{};
        std::vector<VkImageView> swapChainImageViews_;
        VkFormat swapChainImageFormat_ = VK_FORMAT_UNDEFINED;

        std::unique_ptr<RenderPass> worldRenderPass_;

        std::vector<VkImage> depthImages_;
        std::vector<VkDeviceMemory> depthImageMemories_;
        std::vector<VkImageView> depthImageViews_;
        std::vector<VkFramebuffer> framebuffers_;

        VkPipelineLayout pipelineLayout_ = VK_NULL_HANDLE;
        std::unique_ptr<Pipeline> pipeline_;
        std::vector<char> vertShaderCode_;
        std::vector<char> fragShaderCode_;

        TerrainGenerator terrainNoise_;
        std::vector<Chunk> chunks_;

        Camera camera_;
        double lastMouseX_ = 0.0;
        double lastMouseY_ = 0.0;
        bool cursorCaptured_ = false;
    };

} // namespace lve
