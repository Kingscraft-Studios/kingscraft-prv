#include "Renderer/Renderer.hpp"
#include "Vulkan/RenderPass.hpp"
#include <stdexcept>
#include <limits>

namespace lve {

Renderer::Renderer(Device& device, VkExtent2D initialExtent)
    : device_(device), extent_(initialExtent) {
    swapchain_ = std::make_unique<SwapChain>(device_, extent_);
    syncObjects_ = std::make_unique<SyncObjects>(device_, static_cast<uint32_t>(swapchain_->imageCount()));
    framebufferManager_ = std::make_unique<FramebufferManager>(device_);
    presentRenderPass_ = RenderPass::createDefault(device_, swapchain_->getSwapChainImageFormat());
    activeRenderPass_ = presentRenderPass_->getHandle();
    framebufferManager_->createFramebuffers(
        swapchain_->getImageViews(),
        activeRenderPass_,
        swapchain_->getSwapChainExtent());
    imagesInFlight_.resize(swapchain_->imageCount(), VK_NULL_HANDLE);
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

void Renderer::setRenderPass(VkRenderPass renderPass) {
    activeRenderPass_ = renderPass;
    framebufferManager_->createFramebuffers(
        swapchain_->getImageViews(),
        activeRenderPass_,
        swapchain_->getSwapChainExtent());
}

void Renderer::recreateSwapChain(VkExtent2D newExtent) {
    extent_ = newExtent;

    vkDeviceWaitIdle(device_.device());

    auto oldSwapchain = std::move(swapchain_);
    swapchain_ = std::make_unique<SwapChain>(device_, extent_, std::move(oldSwapchain));

    presentRenderPass_ = RenderPass::createDefault(device_, swapchain_->getSwapChainImageFormat());
    activeRenderPass_ = presentRenderPass_->getHandle();

    framebufferManager_->destroyFramebuffers();
    framebufferManager_->createFramebuffers(
        swapchain_->getImageViews(),
        activeRenderPass_,
        swapchain_->getSwapChainExtent());

    imagesInFlight_.resize(swapchain_->imageCount(), VK_NULL_HANDLE);

    if (swapchain_->imageCount() != commandBuffers_.size()) {
        vkFreeCommandBuffers(
            device_.device(), device_.getCommandPool(),
            static_cast<uint32_t>(commandBuffers_.size()), commandBuffers_.data());
        createCommandBuffers();
    }
}

bool Renderer::beginFrame() {
    VkFence inFlightFence = syncObjects_->getInFlight(currentFrame_);
    vkWaitForFences(
        device_.device(),
        1,
        &inFlightFence,
        VK_TRUE,
        std::numeric_limits<uint64_t>::max());

    auto result = swapchain_->acquireNextImage(&currentImageIndex_, syncObjects_->getImageAvailable(currentFrame_));
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

    if (imagesInFlight_[currentImageIndex_] != VK_NULL_HANDLE) {
        vkWaitForFences(device_.device(), 1, &imagesInFlight_[currentImageIndex_], VK_TRUE, UINT64_MAX);
    }
    imagesInFlight_[currentImageIndex_] = syncObjects_->getInFlight(currentFrame_);

    VkSubmitInfo submitInfo{};
    submitInfo.sType = VK_STRUCTURE_TYPE_SUBMIT_INFO;

    VkSemaphore waitSemaphores[] = {syncObjects_->getImageAvailable(currentFrame_)};
    VkPipelineStageFlags waitStages[] = {VK_PIPELINE_STAGE_COLOR_ATTACHMENT_OUTPUT_BIT};
    submitInfo.waitSemaphoreCount = 1;
    submitInfo.pWaitSemaphores = waitSemaphores;
    submitInfo.pWaitDstStageMask = waitStages;

    submitInfo.commandBufferCount = 1;
    submitInfo.pCommandBuffers = &cmd;

    VkSemaphore signalSemaphores[] = {syncObjects_->getRenderFinished(currentImageIndex_)};
    submitInfo.signalSemaphoreCount = 1;
    submitInfo.pSignalSemaphores = signalSemaphores;

    VkFence inFlightFence = syncObjects_->getInFlight(currentFrame_);
    vkResetFences(device_.device(), 1, &inFlightFence);

    if (vkQueueSubmit(device_.graphicsQueue(), 1, &submitInfo, inFlightFence) != VK_SUCCESS) {
        throw std::runtime_error("failed to submit draw command buffer!");
    }

    VkPresentInfoKHR presentInfo{};
    presentInfo.sType = VK_STRUCTURE_TYPE_PRESENT_INFO_KHR;
    presentInfo.waitSemaphoreCount = 1;
    presentInfo.pWaitSemaphores = signalSemaphores;

    VkSwapchainKHR swapChains[] = {swapchain_->getSwapChain()};
    presentInfo.swapchainCount = 1;
    presentInfo.pSwapchains = swapChains;
    presentInfo.pImageIndices = &currentImageIndex_;

    auto result = vkQueuePresentKHR(device_.presentQueue(), &presentInfo);

    currentFrame_ = (currentFrame_ + 1) % SyncObjects::MAX_FRAMES_IN_FLIGHT;

    if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR) {
        return false;
    }
    if (result != VK_SUCCESS) {
        throw std::runtime_error("failed to submit command buffers!");
    }

    return true;
}

} // namespace lve
