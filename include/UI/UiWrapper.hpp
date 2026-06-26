#pragma once

#include <vulkan/vulkan.h>

#include "UI/Engine/UiEngine.hpp"
#include "UI/Elements/UiElement.hpp"
#include "UI/UiStyle.hpp"
#include <functional>
#include <memory>
#include <string>
#include <unordered_map>
#include <vector>
#include <cstdint>

namespace lve {

    class Device;
    class DescriptorManager;


    class UiWrapper {
    public:
        UiWrapper();
        ~UiWrapper();

        void init(Device& device, DescriptorManager& descriptorManager, VkExtent2D extent);
        void shutdown();
        void update(double deltaTime);
        void resize(int width, int height);
        void onMouseMove(double x, double y);
        void onMouseButton(int button, int action, int mods, double x, double y);
        void onScroll(double dx, double dy);
        void onKey(int key, int action);
        void onChar(unsigned int codepoint);

        void renderOffscreen(VkCommandBuffer cmdBuffer);
        void render(VkCommandBuffer cmdBuffer, VkRenderPass renderPass);

        void addElement(UiElement* element);
        void removeElement(UiElement* element);
        void registerButtonHandler(std::string name, std::function<void()> handler);

        // Style system
        uint32_t registerStyle(const UiStyle& style);
        void updateStylePool();
        void markDirty(uint32_t elementId);

    private:
        bool initialized_ = false;
        int width_ = 0;
        int height_ = 0;
        uint64_t frameIndex_ = 0;

        std::unique_ptr<UiEngine> engine_;
        std::vector<UiElement*> elements_;

        VkRenderPass currentRenderPass_ = VK_NULL_HANDLE;
        std::unordered_map<std::string, std::function<void()>> buttonHandlers_;

        // Style system
        uint32_t nextElementId_ = 0;
        std::vector<GpuStyle> stylePool_;
        std::vector<uint32_t> elementStyles_;
        std::vector<bool> styleDirty_;

        // Dirty range tracking
        uint32_t firstDirty_ = UiRenderer::MAX_ELEMENTS;
        uint32_t lastDirty_ = 0;
        bool stylePoolDirty_ = false;
    };

} // namespace lve
