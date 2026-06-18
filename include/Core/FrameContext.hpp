#pragma once

#include <vulkan/vulkan.h>

namespace lve {

struct FrameContext {
    VkCommandBuffer cmd = VK_NULL_HANDLE;
    VkRenderPass renderPass = VK_NULL_HANDLE;
    VkExtent2D extent{};

    double dt = 0.0;
    uint32_t frameIndex = 0;
    uint32_t imageIndex = 0;
};

} // namespace lve
