#include "Vulkan/SyncObjects.hpp"
#include <stdexcept>

namespace lve {

    SyncObjects::SyncObjects(Device& device, uint32_t swapchainImageCount)
        : device_(device) {

        imageAvailable_.resize(MAX_FRAMES_IN_FLIGHT);
        renderFinished_.resize(swapchainImageCount);
        inFlightFences_.resize(MAX_FRAMES_IN_FLIGHT);

        VkSemaphoreCreateInfo semaphoreInfo{};
        semaphoreInfo.sType = VK_STRUCTURE_TYPE_SEMAPHORE_CREATE_INFO;

        VkFenceCreateInfo fenceInfo{};
        fenceInfo.sType = VK_STRUCTURE_TYPE_FENCE_CREATE_INFO;
        fenceInfo.flags = VK_FENCE_CREATE_SIGNALED_BIT;

        for (size_t i = 0; i < MAX_FRAMES_IN_FLIGHT; i++) {
            if (vkCreateSemaphore(device_.device(), &semaphoreInfo, nullptr, &imageAvailable_[i]) != VK_SUCCESS ||
                vkCreateFence(device_.device(), &fenceInfo, nullptr, &inFlightFences_[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create sync objects for a frame!");
            }
        }

        for (size_t i = 0; i < swapchainImageCount; i++) {
            if (vkCreateSemaphore(device_.device(), &semaphoreInfo, nullptr, &renderFinished_[i]) != VK_SUCCESS) {
                throw std::runtime_error("failed to create per-image render finished semaphore!");
            }
        }
    }

    SyncObjects::~SyncObjects() {
        for (auto sem : imageAvailable_) {
            vkDestroySemaphore(device_.device(), sem, nullptr);
        }
        for (auto sem : renderFinished_) {
            vkDestroySemaphore(device_.device(), sem, nullptr);
        }
        for (auto fence : inFlightFences_) {
            vkDestroyFence(device_.device(), fence, nullptr);
        }
    }

} // namespace lve
