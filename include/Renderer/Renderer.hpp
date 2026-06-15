#pragma once

#include "Vulkan/Device.hpp"
#include "Vulkan/Swapchain.hpp"
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
    VkRenderPass getSwapChainRenderPass() const { return swapchain_->getRenderPass(); }
    VkFramebuffer getCurrentFramebuffer() const { return swapchain_->getFrameBuffer(currentImageIndex_); }
    uint32_t getCurrentImageIndex() const { return currentImageIndex_; }
    Device& getDevice() { return device_; }

private:
    void createCommandBuffers();

    Device& device_;
    VkExtent2D extent_;
    std::unique_ptr<SwapChain> swapchain_;
    std::vector<VkCommandBuffer> commandBuffers_;
    uint32_t currentImageIndex_ = 0;
};

} // namespace lve
