#pragma once

#include "Vulkan/Device.hpp"
#include "Vulkan/Swapchain.hpp"
#include "Vulkan/SyncObjects.hpp"
#include "Vulkan/FramebufferManager.hpp"
#include "Vulkan/RenderPass.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace lve {

struct RenderPassBegin {
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkFramebuffer framebuffer = VK_NULL_HANDLE;
    VkRect2D renderArea{};
    VkViewport viewport{};
    VkRect2D scissor{};
    std::vector<VkClearValue> clearValues;
};

class Renderer {
public:
    Renderer(Device& device, VkExtent2D initialExtent);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void recreateSwapChain(VkExtent2D newExtent);

    bool beginFrame();
    void executeRenderPass(
        const RenderPassBegin& passBegin,
        const std::function<void(VkCommandBuffer)>& drawCommands);
    bool endFrame();

    VkCommandBuffer getActiveCommandBuffer() const { return commandBuffers_[currentImageIndex_]; }
    VkExtent2D getExtent() const { return swapchain_->getSwapChainExtent(); }
    VkFramebuffer getCurrentFramebuffer() const { return framebufferManager_->getFramebuffer(currentImageIndex_); }
    uint32_t getCurrentImageIndex() const { return currentImageIndex_; }
    Device& getDevice() { return device_; }

    VkFormat getSwapChainImageFormat() const { return swapchain_->getSwapChainImageFormat(); }
    uint32_t getSwapChainImageCount() const { return static_cast<uint32_t>(swapchain_->imageCount()); }
    const std::vector<VkImageView>& getSwapChainImageViews() const { return swapchain_->getImageViews(); }
    SwapChain& getSwapChain() { return *swapchain_; }

    VkRenderPass getRenderPass() const { return presentRenderPass_->getHandle(); }
    uint32_t getFrameIndex() const { return currentFrame_; }

    VkRenderPass getWorldRenderPass() const { return worldRenderPass_->getHandle(); }
    VkFramebuffer getWorldFramebuffer(uint32_t imageIndex) const { return worldFramebuffers_[imageIndex]; }
    VkFormat getDepthFormat() const { return depthFormat_; }
    double getGpuFrameTimeMs() const { return gpuFrameTimeMs_; }

private:
    void createCommandBuffers();
    void createWorldResources();
    void destroyWorldResources();
    void createQueryPool();

    Device& device_;
    VkExtent2D extent_;
    std::unique_ptr<SwapChain> swapchain_;
    std::unique_ptr<SyncObjects> syncObjects_;
    std::unique_ptr<FramebufferManager> framebufferManager_;
    std::unique_ptr<RenderPass> presentRenderPass_;

    VkFormat depthFormat_ = VK_FORMAT_UNDEFINED;
    std::unique_ptr<RenderPass> worldRenderPass_;
    std::vector<VkImage> depthImages_;
    std::vector<VkDeviceMemory> depthImageMemories_;
    std::vector<VkImageView> depthImageViews_;
    std::vector<VkFramebuffer> worldFramebuffers_;

    std::vector<VkCommandBuffer> commandBuffers_;
    std::vector<VkFence> imagesInFlight_;
    uint32_t currentFrame_ = 0;
    uint32_t currentImageIndex_ = 0;

    VkQueryPool gpuQueryPool_ = VK_NULL_HANDLE;
    double timestampPeriod_ = 1.0;
    double gpuFrameTimeMs_ = 0.0;
};

} // namespace lve
