#include "UI/Engine/UiEngine.hpp"

namespace lve {

    UiEngine::UiEngine(Device& device, DescriptorManager& descriptorManager, VkExtent2D extent)
        : device_(device),
          descriptorManager_(descriptorManager),
          extent_(extent) {
    }

    UiEngine::~UiEngine() {
        shutdown();
    }

    void UiEngine::init() {
        if (initialized_) return;

        batchQueue_ = std::make_unique<UiBatchQueue>(device_);
        fontAtlas_ = std::make_unique<UiFontAtlas>(device_);
        renderer_ = std::make_unique<UiRenderer>(device_, descriptorManager_, extent_);

        renderer_->init();
        initialized_ = true;
    }

    void UiEngine::shutdown() {
        if (!initialized_) return;

        renderer_->shutdown();
        renderer_.reset();
        fontAtlas_.reset();
        batchQueue_.reset();

        initialized_ = false;
    }

    void UiEngine::resize(VkExtent2D extent) {
        extent_ = extent;
        if (initialized_) {
            renderer_->resize(extent);
        }
    }

    void UiEngine::beginFrame() {
        batchQueue_->begin();
    }

    void UiEngine::renderOffscreen(VkCommandBuffer cmd) {
        if (!initialized_) return;

        if (fontAtlas_->isReady() && !renderer_->hasFontAtlas()) {
            renderer_->setFontAtlas(fontAtlas_->getImageView(), fontAtlas_->getSampler());
        }

        renderer_->renderOffscreen(cmd, *batchQueue_);
    }

    void UiEngine::composite(VkCommandBuffer cmd) {
        if (!initialized_) return;
        renderer_->composite(cmd);
    }

    void UiEngine::addQuad(const UiVertex verts[4], const uint32_t indices[6]) {
        batchQueue_->addQuad(verts, indices);
    }

    bool UiEngine::loadFont(const std::string& ttfPath, const std::string& name) {
        return fontAtlas_->loadFont(ttfPath, name);
    }

    const UiFontAtlas::Glyph* UiEngine::getGlyph(const std::string& font, char32_t codepoint) const {
        return fontAtlas_->getGlyph(font, codepoint);
    }

} // namespace lve
