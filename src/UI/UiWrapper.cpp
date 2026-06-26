// TODO: SSBO Style System + Debug Editing Mode
// UiWrapper orchestrates:
// 1. RegisterStyle() — adds UiStyle to stylePool_, returns index
// 2. addElement() — assigns elementId, initializes elementStyles_ entry
// 3. renderOffscreen() — uploads dirty-range elementStyles_ to GPU,
//    then iterates elements calling render()
// 4. Dirty tracking: button hover changes mark single element dirty
// 5. Debug mode (F3+F6): click-drag reposition elements, F7 logs anchor code

#include "UI/UiWrapper.hpp"
#include "UI/Elements/UiButton.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/DescriptorManager.hpp"
#include "Threads/Logger.hpp"
#include "Util/StringBuilder.hpp"
#include <algorithm>
#include <cmath>

namespace lve {

    UiWrapper::UiWrapper() {
        stylePool_.reserve(UiRenderer::MAX_STYLES);
        elementStyles_.resize(UiRenderer::MAX_ELEMENTS, 0);
        styleDirty_.resize(UiRenderer::MAX_ELEMENTS, false);
    }

    UiWrapper::~UiWrapper() {
        shutdown();
    }

    void UiWrapper::init(Device& device, DescriptorManager& descriptorManager, VkExtent2D extent) {
        if (initialized_) return;

        width_ = static_cast<int>(extent.width);
        height_ = static_cast<int>(extent.height);

        engine_ = std::make_unique<UiEngine>(device, descriptorManager, extent);
        engine_->init();

        engine_->loadFont("resources/fonts/Arial.ttf", "default");

        initialized_ = true;
    }

    void UiWrapper::shutdown() {
        if (!initialized_) return;
        elements_.clear();
        engine_->shutdown();
        engine_.reset();
        initialized_ = false;
    }

    void UiWrapper::update(double deltaTime) {
        (void)deltaTime;
    }

    void UiWrapper::resize(int width, int height) {
        width_ = width;
        height_ = height;
        if (engine_) {
            engine_->resize({static_cast<uint32_t>(width), static_cast<uint32_t>(height)});
        }
        for (auto* element : elements_) {
            element->updateLayout(static_cast<float>(width), static_cast<float>(height));
        }
    }

    void UiWrapper::onMouseMove(double x, double y) {
        if (!engine_) return;
        glm::vec2 point(static_cast<float>(x), static_cast<float>(y));

        glm::vec2 delta = point - lastMouse_;
        lastMouse_ = point;

        if (debugMode_) {
            if (isDragging_ && selectedElement_) {
                selectedElement_->adjustPixelOffset(delta.x, delta.y);
                selectedElement_->updateLayout(
                    static_cast<float>(width_), static_cast<float>(height_));
                markDirty(selectedElement_->getElementId());
                return;
            }

            // Find topmost element under cursor for hover highlight
            hoveredElement_ = nullptr;
            for (auto it = elements_.rbegin(); it != elements_.rend(); ++it) {
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
            return;
        }

        // Normal mode: button hover state updates
        for (auto* element : elements_) {
            auto* button = dynamic_cast<UiButton*>(element);
            if (button) {
                bool prevHover = button->isHovered();
                button->setHovered(button->containsPoint(point));
                if (prevHover != button->isHovered()) {
                    markDirty(button->getElementId());
                }
            }
        }
    }

    void UiWrapper::onMouseButton(int button, int action, int mods, double x, double y) {
        (void)mods;
        if (!engine_) return;

        if (debugMode_) {
            if (button != 0) return;

            glm::vec2 point(static_cast<float>(x), static_cast<float>(y));

            if (action == 1) {  // Press
                isDragging_ = false;
                selectedElement_ = nullptr;

                // Find topmost element under cursor (skip debug overlay rects, skip fill-parent)
                for (auto it = elements_.rbegin(); it != elements_.rend(); ++it) {
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
                    isDragging_ = false;
                    if (len < 3.0f) {
                        // Click (no drag) — keep element selected
                    } else {
                        // Drag completed — element already moved by onMouseMove
                    }
                }
            }
            return;
        }

        // Normal mode: button click handling
        if (button != 0 || action != 1) return;

        glm::vec2 point(static_cast<float>(x), static_cast<float>(y));

        for (auto it = elements_.rbegin(); it != elements_.rend(); ++it) {
            auto* btn = dynamic_cast<UiButton*>(*it);
            if (btn && btn->containsPoint(point)) {
                btn->click();
                std::string name = btn->getHandlerName();
                if (!name.empty()) {
                    auto handlerIt = buttonHandlers_.find(name);
                    if (handlerIt != buttonHandlers_.end()) {
                        handlerIt->second();
                    }
                }
                return;
            }
        }
    }

    void UiWrapper::onScroll(double dx, double dy) {
        (void)dx; (void)dy;
    }

    void UiWrapper::onKey(int key, int action) {
        (void)key; (void)action;
    }

    void UiWrapper::onChar(unsigned int codepoint) {
        (void)codepoint;
    }

    void UiWrapper::renderOffscreen(VkCommandBuffer cmdBuffer) {
        if (!engine_) return;

        engine_->beginFrame();

        // Upload style pool if dirty
        if (stylePoolDirty_ && !stylePool_.empty()) {
            engine_->uploadStylePool(stylePool_.data(),
                static_cast<uint32_t>(stylePool_.size()));
            stylePoolDirty_ = false;
        }

        // Upload dirty element style indices in range [firstDirty_, lastDirty_)
        if (firstDirty_ < lastDirty_) {
            uint32_t uploadCount = lastDirty_ - firstDirty_;
            engine_->uploadElementStyles(
                &elementStyles_[firstDirty_],
                firstDirty_,
                uploadCount);
            for (uint32_t i = firstDirty_; i < lastDirty_; i++) {
                styleDirty_[i] = false;
            }
            firstDirty_ = UiRenderer::MAX_ELEMENTS;
            lastDirty_ = 0;
        }

        for (auto* element : elements_) {
            element->render(*engine_);
        }

        if (debugMode_) {
            renderDebugOverlays();
        }

        engine_->renderOffscreen(cmdBuffer);
    }

    void UiWrapper::render(VkCommandBuffer cmdBuffer, VkRenderPass renderPass) {
        if (!engine_) return;

        if (renderPass != currentRenderPass_) {
            currentRenderPass_ = renderPass;
            engine_->getRenderer().setTargetRenderPass(renderPass);
        }

        engine_->composite(cmdBuffer);
    }

    void UiWrapper::addElement(UiElement* element) {
        uint32_t id = nextElementId_++;
        element->setElementId(id);
        if (id < UiRenderer::MAX_ELEMENTS) {
            elementStyles_[id] = element->getStyleIndex();
            styleDirty_[id] = true;
            firstDirty_ = std::min(firstDirty_, id);
            lastDirty_ = std::max(lastDirty_, id + 1);
        }
        elements_.push_back(element);
    }

    void UiWrapper::removeElement(UiElement* element) {
        auto it = std::find(elements_.begin(), elements_.end(), element);
        if (it != elements_.end()) {
            elements_.erase(it);
        }
        if (selectedElement_ == element) selectedElement_ = nullptr;
        if (hoveredElement_ == element) hoveredElement_ = nullptr;
    }

    void UiWrapper::registerButtonHandler(std::string name, std::function<void()> handler) {
        buttonHandlers_[name] = std::move(handler);
    }

    uint32_t UiWrapper::registerStyle(const UiStyle& style) {
        uint32_t index = static_cast<uint32_t>(stylePool_.size());
        stylePool_.push_back(style.toGpu());
        stylePoolDirty_ = true;
        return index;
    }

    void UiWrapper::updateStylePool() {
        stylePoolDirty_ = true;
    }

    void UiWrapper::markDirty(uint32_t elementId) {
        if (elementId >= UiRenderer::MAX_ELEMENTS) return;
        for (auto* element : elements_) {
            if (element->getElementId() == elementId) {
                elementStyles_[elementId] = element->getActiveStyleIndex();
                break;
            }
        }
        if (!styleDirty_[elementId]) {
            styleDirty_[elementId] = true;
            firstDirty_ = std::min(firstDirty_, elementId);
            lastDirty_ = std::max(lastDirty_, elementId + 1);
        }
    }

    // ---- Debug Editing Mode ----

    void UiWrapper::toggleDebugMode() {
        debugMode_ = !debugMode_;
        if (debugMode_) {
            ensureDebugOverlay();
            selectedElement_ = nullptr;
            hoveredElement_ = nullptr;
            isDragging_ = false;
        } else {
            selectedElement_ = nullptr;
            hoveredElement_ = nullptr;
            isDragging_ = false;
        }
        Logger::Get().log(LogLevel::INFO, ThreadName::Engine,
            StringBuilder::build("[Debug] Editing mode: ",
                debugMode_ ? "ON" : "OFF"));
    }

    void UiWrapper::logSelectedElementPosition() {
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

    void UiWrapper::ensureDebugOverlay() {
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
        uint32_t nextId = nextElementId_;
        nextElementId_ += 2;
        debugElementHoverId_ = nextId;
        debugElementSelectId_ = nextId + 1;

        if (debugElementHoverId_ < UiRenderer::MAX_ELEMENTS) {
            elementStyles_[debugElementHoverId_] = debugHoverStyle_;
            styleDirty_[debugElementHoverId_] = true;
            firstDirty_ = std::min(firstDirty_, debugElementHoverId_);
            lastDirty_ = std::max(lastDirty_, debugElementHoverId_ + 1);
        }
        if (debugElementSelectId_ < UiRenderer::MAX_ELEMENTS) {
            elementStyles_[debugElementSelectId_] = debugBorderStyle_;
            styleDirty_[debugElementSelectId_] = true;
            firstDirty_ = std::min(firstDirty_, debugElementSelectId_);
            lastDirty_ = std::max(lastDirty_, debugElementSelectId_ + 1);
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

    void UiWrapper::renderDebugOverlays() {
        if (hoveredElement_ && hoveredElement_ != selectedElement_) {
            debugHoverRect_.setPosition(hoveredElement_->getPosition());
            debugHoverRect_.setSize(hoveredElement_->getSize());
            debugHoverRect_.render(*engine_);
        }

        if (selectedElement_) {
            glm::vec2 pos = selectedElement_->getPosition();
            glm::vec2 sz = selectedElement_->getSize();
            float bw = 2.0f;

            // Top
            debugBorderTop_.setPosition(pos);
            debugBorderTop_.setSize({sz.x, bw});
            debugBorderTop_.render(*engine_);
            // Bottom
            debugBorderBottom_.setPosition({pos.x, pos.y + sz.y - bw});
            debugBorderBottom_.setSize({sz.x, bw});
            debugBorderBottom_.render(*engine_);
            // Left
            debugBorderLeft_.setPosition(pos);
            debugBorderLeft_.setSize({bw, sz.y});
            debugBorderLeft_.render(*engine_);
            // Right
            debugBorderRight_.setPosition({pos.x + sz.x - bw, pos.y});
            debugBorderRight_.setSize({bw, sz.y});
            debugBorderRight_.render(*engine_);
        }
    }

} // namespace lve
