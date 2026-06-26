#pragma once

#include "Vulkan/Device.hpp"
#include "Vulkan/DescriptorManager.hpp"
#include "UI/Engine/UiBatchQueue.hpp"
#include "UI/Engine/UiFontAtlas.hpp"
#include "UI/Engine/UiRenderer.hpp"
#include <glm/glm.hpp>
#include <memory>
#include <string>

namespace lve {

    class UiEngine {
    public:
        UiEngine(Device& device, DescriptorManager& descriptorManager, VkExtent2D extent);
        ~UiEngine();

        UiEngine(const UiEngine&) = delete;
        UiEngine& operator=(const UiEngine&) = delete;

        void init();
        void shutdown();
        void resize(VkExtent2D extent);

        // Frame lifecycle
        void beginFrame();
        void renderOffscreen(VkCommandBuffer cmd);
        void composite(VkCommandBuffer cmd);

        // Draw commands
        void addQuad(const UiVertex verts[4], const uint32_t indices[6]);

        // Font access
        bool loadFont(const std::string& ttfPath, const std::string& name);
        const UiFontAtlas::Glyph* getGlyph(const std::string& font, char32_t codepoint) const;

        // Style SSBO upload
        void uploadStylePool(const void* data, uint32_t count);
        void uploadElementStyles(const void* data, uint32_t firstElement, uint32_t count);

        // Subsystem access (for advanced use)
        UiFontAtlas& getFontAtlas() { return *fontAtlas_; }
        UiBatchQueue& getBatchQueue() { return *batchQueue_; }
        UiRenderer& getRenderer() { return *renderer_; }

    private:
        Device& device_;
        DescriptorManager& descriptorManager_;
        VkExtent2D extent_;

        std::unique_ptr<UiBatchQueue> batchQueue_;
        std::unique_ptr<UiFontAtlas> fontAtlas_;
        std::unique_ptr<UiRenderer> renderer_;

        bool initialized_ = false;
    };

} // namespace lve
