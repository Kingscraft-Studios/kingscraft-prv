#pragma once

#include "Vulkan/Device.hpp"
#include "Vulkan/Swapchain.hpp"
#include "Vulkan/SyncObjects.hpp"
#include "Vulkan/FramebufferManager.hpp"
#include <memory>
#include <vector>
#include <functional>

namespace lve { class RenderPass; }

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
    SwapChain& getSwapChain() { return *swapchain_; }

    VkRenderPass getRenderPass() const { return activeRenderPass_; }
    uint32_t getFrameIndex() const { return currentFrame_; }
    void setRenderPass(VkRenderPass renderPass);

private:
    void createCommandBuffers();

    Device& device_;
    VkExtent2D extent_;
    std::unique_ptr<SwapChain> swapchain_;
    std::unique_ptr<SyncObjects> syncObjects_;
    std::unique_ptr<FramebufferManager> framebufferManager_;
    std::unique_ptr<RenderPass> presentRenderPass_;
    VkRenderPass activeRenderPass_ = VK_NULL_HANDLE;
    std::vector<VkCommandBuffer> commandBuffers_;
    std::vector<VkFence> imagesInFlight_;
    uint32_t currentFrame_ = 0;
    uint32_t currentImageIndex_ = 0;
};

} // namespace lve
