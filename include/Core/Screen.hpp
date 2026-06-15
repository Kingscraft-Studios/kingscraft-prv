#pragma once

#include <vulkan/vulkan.h>

namespace lve {

    class Screen {
    public:
        virtual ~Screen() = default;

        virtual void init() = 0;
        virtual void tick(double dt) = 0;
        virtual void render(VkCommandBuffer commandBuffer) = 0;
        virtual void cleanup() = 0;
    };

} // namespace lve
