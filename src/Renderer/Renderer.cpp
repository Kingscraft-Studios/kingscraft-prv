#include "Renderer/Renderer.hpp"
#include <stdexcept>

namespace lve {

Renderer::Renderer(Device& device, VkExtent2D initialExtent)
    : device_(device), extent_(initialExtent) {
    swapchain_ = std::make_unique<SwapChain>(device_, extent_);
    createCommandBuffers();
}

Renderer::~Renderer() {
    vkFreeCommandBuffers(
        device_.device(), device_.getCommandPool(),
        static_cast<uint32_t>(commandBuffers_.size()), commandBuffers_.data());
}

void Renderer::createCommandBuffers() {
    commandBuffers_.resize(swapchain_->imageCount());

    VkCommandBufferAllocateInfo allocInfo{};
    allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
    allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
    allocInfo.commandPool = device_.getCommandPool();
    allocInfo.commandBufferCount = static_cast<uint32_t>(commandBuffers_.size());

    if (vkAllocateCommandBuffers(device_.device(), &allocInfo, commandBuffers_.data()) != VK_SUCCESS) {
        throw std::runtime_error("failed to allocate command buffers!");
    }
}

void Renderer::recreateSwapChain(VkExtent2D newExtent) {
    extent_ = newExtent;

    auto oldSwapchain = std::move(swapchain_);
    swapchain_ = std::make_unique<SwapChain>(device_, extent_, std::move(oldSwapchain));

    if (swapchain_->imageCount() != commandBuffers_.size()) {
        vkFreeCommandBuffers(
            device_.device(), device_.getCommandPool(),
            static_cast<uint32_t>(commandBuffers_.size()), commandBuffers_.data());
        createCommandBuffers();
    }
}

bool Renderer::beginFrame() {
    auto result = swapchain_->acquireNextImage(&currentImageIndex_);
    if (result == VK_ERROR_OUT_OF_DATE_KHR) {
        return false;
    }
    if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR) {
        throw std::runtime_error("failed to acquire swap chain image!");
    }

    VkCommandBufferBeginInfo beginInfo{};
    beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

    if (vkBeginCommandBuffer(commandBuffers_[currentImageIndex_], &beginInfo) != VK_SUCCESS) {
        throw std::runtime_error("failed to begin recording command buffer!");
    }

    return true;
}

void Renderer::executeRenderPass(
    const RenderPassBegin& passBegin,
    const std::function<void(VkCommandBuffer)>& drawCommands) {

    VkCommandBuffer cmd = commandBuffers_[currentImageIndex_];

    VkRenderPassBeginInfo rpInfo{};
    rpInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
    rpInfo.renderPass = passBegin.renderPass;
    rpInfo.framebuffer = passBegin.framebuffer;
    rpInfo.renderArea = passBegin.renderArea;
    rpInfo.clearValueCount = static_cast<uint32_t>(passBegin.clearValues.size());
    rpInfo.pClearValues = passBegin.clearValues.data();

    vkCmdBeginRenderPass(cmd, &rpInfo, VK_SUBPASS_CONTENTS_INLINE);
    vkCmdSetViewport(cmd, 0, 1, &passBegin.viewport);
    vkCmdSetScissor(cmd, 0, 1, &passBegin.scissor);
    drawCommands(cmd);
    vkCmdEndRenderPass(cmd);
}

bool Renderer::endFrame() {
    VkCommandBuffer cmd = commandBuffers_[currentImageIndex_];

    if (vkEndCommandBuffer(cmd) != VK_SUCCESS) {
        throw std::runtime_error("failed to record command buffer!");
    }

    auto result = swapchain_->submitCommandBuffers(&cmd, &currentImageIndex_);
    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return false;
    }
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to submit command buffers!");
    }

    return true;
}

} // namespace lve
