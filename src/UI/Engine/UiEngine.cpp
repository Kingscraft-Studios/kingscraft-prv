#include "UI/Engine/UiEngine.hpp"
#include "Threads/Logger.hpp"
#include "Util/StringBuilder.hpp"

namespace lve {

    UiEngine::UiEngine(Device& device, DescriptorManager& descriptorManager, VkExtent2D extent)
        : device_(device),
          descriptorManager_(descriptorManager),
          extent_(extent) {
        stylePool_.reserve(UiRenderer::MAX_STYLES);
        elementStyles_.resize(UiRenderer::MAX_ELEMENTS, 0);
        styleDirty_.resize(UiRenderer::MAX_ELEMENTS, false);
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

        uploadPendingStyles();

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

    void UiEngine::uploadStylePool(const void* data, uint32_t count) {
        if (initialized_) renderer_->uploadStylePool(data, count);
    }

    void UiEngine::uploadElementStyles(const void* data, uint32_t firstElement, uint32_t count) {
        if (initialized_) renderer_->uploadElementStyles(data, firstElement, count);
    }

    // ---- Style System ----

    uint32_t UiEngine::registerStyle(const UiStyle& style) {
        uint32_t index = static_cast<uint32_t>(stylePool_.size());
        stylePool_.push_back(style.toGpu());
        stylePoolDirty_ = true;
        return index;
    }

    void UiEngine::updateStylePool() {
        stylePoolDirty_ = true;
    }

    void UiEngine::markDirty(uint32_t elementId) {
        if (elementId >= UiRenderer::MAX_ELEMENTS) return;
        if (!styleDirty_[elementId]) {
            styleDirty_[elementId] = true;
            firstDirty_ = std::min(firstDirty_, elementId);
            lastDirty_ = std::max(lastDirty_, elementId + 1);
        }
    }

    void UiEngine::setElementStyle(uint32_t id, uint32_t styleIndex) {
        if (id < UiRenderer::MAX_ELEMENTS) {
            elementStyles_[id] = styleIndex;
            styleDirty_[id] = true;
            firstDirty_ = std::min(firstDirty_, id);
            lastDirty_ = std::max(lastDirty_, id + 1);
        }
    }

    uint32_t UiEngine::allocateElementId() {
        return nextElementId_++;
    }

    void UiEngine::uploadPendingStyles() {
        if (stylePoolDirty_ && !stylePool_.empty()) {
            uploadStylePool(stylePool_.data(),
                static_cast<uint32_t>(stylePool_.size()));
            stylePoolDirty_ = false;
        }

        if (firstDirty_ < lastDirty_) {
            uint32_t uploadCount = lastDirty_ - firstDirty_;
            uploadElementStyles(
                &elementStyles_[firstDirty_],
                firstDirty_,
                uploadCount);
            for (uint32_t i = firstDirty_; i < lastDirty_; i++) {
                styleDirty_[i] = false;
            }
            firstDirty_ = UiRenderer::MAX_ELEMENTS;
            lastDirty_ = 0;
        }
    }

    // ---- Debug Editing Mode ----

    void UiEngine::setDebugMode(bool on) {
        debugMode_ = on;
        if (debugMode_) {
            ensureDebugOverlay();
        }
        selectedElement_ = nullptr;
        hoveredElement_ = nullptr;
        isDragging_ = false;
        Logger::Get().log(LogLevel::INFO, ThreadName::Engine,
            StringBuilder::build("[Debug] Editing mode: ",
                debugMode_ ? "ON" : "OFF"));
    }

    void UiEngine::logSelectedElementPosition() {
        if (!selectedElement_) {
            Logger::Get().log(LogLevel::INFO, ThreadName::Engine,
                "[Debug] No element selected. Click an element first.");
            return;
        }

        glm::vec2 norm = selectedElement_->getNormalizedPos();
        glm::vec2 off = selectedElement_->getPixelOffset();
        glm::vec2 sz = selectedElement_->getSize();

        Logger::Get().log(LogLevel::INFO, ThreadName::Engine,
            StringBuilder::build("[Debug] --- Debug Position ---"));
        Logger::Get().log(LogLevel::INFO, ThreadName::Engine,
            StringBuilder::build("[Debug] Element: ", selectedElement_->getName()));
        Logger::Get().log(LogLevel::INFO, ThreadName::Engine,
            StringBuilder::build("[Debug] .setAnchor({",
                norm.x, "f, ", norm.y, "f}, {",
                off.x, "f, ", off.y, "f});"));
        Logger::Get().log(LogLevel::INFO, ThreadName::Engine,
            StringBuilder::build("[Debug] .setSize({",
                sz.x, "f, ", sz.y, "f});"));
        Logger::Get().log(LogLevel::INFO, ThreadName::Engine,
            StringBuilder::build("[Debug] -------------------------"));
    }

    void UiEngine::handleDebugMouseMove(float x, float y, const std::vector<UiElement*>& elements, float parentW, float parentH) {
        glm::vec2 point(x, y);
        glm::vec2 delta = point - lastMouse_;
        lastMouse_ = point;

        if (isDragging_ && selectedElement_) {
            selectedElement_->adjustPixelOffset(delta.x, delta.y);
            selectedElement_->updateLayout(parentW, parentH);
            markDirty(selectedElement_->getElementId());
            return;
        }

        // Find topmost element under cursor for hover highlight
        hoveredElement_ = nullptr;
        for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
            UiElement* el = *it;
            if (el == &debugHoverRect_) continue;
            if (el == &debugBorderTop_ || el == &debugBorderBottom_ ||
                el == &debugBorderLeft_ || el == &debugBorderRight_) continue;
            if (el->getNormalizedSize().x >= 0.999f && el->getNormalizedSize().y >= 0.999f) continue;
            if (el->containsPoint(point)) {
                hoveredElement_ = el;
                break;
            }
        }
    }

    void UiEngine::handleDebugMouseButton(int button, int action, float x, float y, const std::vector<UiElement*>& elements) {
        if (button != 0) return;

        glm::vec2 point(x, y);

        if (action == 1) {  // Press
            isDragging_ = false;
            selectedElement_ = nullptr;

            // Find topmost element under cursor (skip debug overlay rects, skip fill-parent)
            for (auto it = elements.rbegin(); it != elements.rend(); ++it) {
                UiElement* el = *it;
                if (el == &debugHoverRect_) continue;
                if (el == &debugBorderTop_ || el == &debugBorderBottom_ ||
                    el == &debugBorderLeft_ || el == &debugBorderRight_) continue;
                if (el->getNormalizedSize().x >= 0.999f && el->getNormalizedSize().y >= 0.999f) continue;
                if (el->containsPoint(point)) {
                    selectedElement_ = el;
                    isDragging_ = true;
                    dragStartMouse_ = point;
                    break;
                }
            }
        } else {  // Release
            if (isDragging_) {
                glm::vec2 dist = point - dragStartMouse_;
                float len = std::sqrt(dist.x * dist.x + dist.y * dist.y);
                (void)len;
                isDragging_ = false;
            }
        }
    }

    void UiEngine::renderDebugOverlays(const std::vector<UiElement*>& elements) {
        if (hoveredElement_ && hoveredElement_ != selectedElement_) {
            debugHoverRect_.setPosition(hoveredElement_->getPosition());
            debugHoverRect_.setSize(hoveredElement_->getSize());
            debugHoverRect_.render(*this);
        }

        if (selectedElement_) {
            glm::vec2 pos = selectedElement_->getPosition();
            glm::vec2 sz = selectedElement_->getSize();
            float bw = 2.0f;

            // Top
            debugBorderTop_.setPosition(pos);
            debugBorderTop_.setSize({sz.x, bw});
            debugBorderTop_.render(*this);
            // Bottom
            debugBorderBottom_.setPosition({pos.x, pos.y + sz.y - bw});
            debugBorderBottom_.setSize({sz.x, bw});
            debugBorderBottom_.render(*this);
            // Left
            debugBorderLeft_.setPosition(pos);
            debugBorderLeft_.setSize({bw, sz.y});
            debugBorderLeft_.render(*this);
            // Right
            debugBorderRight_.setPosition({pos.x + sz.x - bw, pos.y});
            debugBorderRight_.setSize({bw, sz.y});
            debugBorderRight_.render(*this);
        }
    }

    void UiEngine::onElementRemoved(UiElement* element) {
        if (selectedElement_ == element) selectedElement_ = nullptr;
        if (hoveredElement_ == element) hoveredElement_ = nullptr;
    }

    void UiEngine::ensureDebugOverlay() {
        if (debugOverlayReady_) return;

        // Hover: black 20% alpha tint
        debugHoverStyle_ = registerStyle(UiStyle{
            .mode = RenderMode::Solid,
            .color1 = {0.0f, 0.0f, 0.0f, 0.20f}
        });

        // Border: black 50% alpha
        debugBorderStyle_ = registerStyle(UiStyle{
            .mode = RenderMode::Solid,
            .color1 = {0.0f, 0.0f, 0.0f, 0.50f}
        });

        // Allocate element IDs for debug overlay rects
        debugElementHoverId_ = allocateElementId();
        debugElementSelectId_ = allocateElementId();

        if (debugElementHoverId_ < UiRenderer::MAX_ELEMENTS) {
            setElementStyle(debugElementHoverId_, debugHoverStyle_);
        }
        if (debugElementSelectId_ < UiRenderer::MAX_ELEMENTS) {
            setElementStyle(debugElementSelectId_, debugBorderStyle_);
        }

        // Hover overlay
        debugHoverRect_.setElementId(debugElementHoverId_);
        debugHoverRect_.setStyleIndex(debugHoverStyle_);

        // Selection border: 4 thin rects
        auto setupBorderRect = [&](UiRect& rect) {
            rect.setElementId(debugElementSelectId_);
            rect.setStyleIndex(debugBorderStyle_);
        };

        setupBorderRect(debugBorderTop_);
        setupBorderRect(debugBorderBottom_);
        setupBorderRect(debugBorderLeft_);
        setupBorderRect(debugBorderRight_);

        debugOverlayReady_ = true;
    }

} // namespace lve
