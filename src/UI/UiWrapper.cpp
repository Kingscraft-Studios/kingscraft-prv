// TODO: SSBO Style System
// UiWrapper orchestrates:
// 1. RegisterStyle() — adds UiStyle to stylePool_, returns index
// 2. addElement() — assigns elementId, initializes elementStyles_ entry
// 3. renderOffscreen() — uploads dirty-range elementStyles_ to GPU,
//    then iterates elements calling render()
// 4. Dirty tracking: button hover changes mark single element dirty

#include "UI/UiWrapper.hpp"
#include "UI/Elements/UiButton.hpp"
#include "Vulkan/Device.hpp"
#include "Vulkan/DescriptorManager.hpp"
#include <algorithm>

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
    }

    void UiWrapper::onMouseMove(double x, double y) {
        if (!engine_) return;
        glm::vec2 point(static_cast<float>(x), static_cast<float>(y));

        for (auto* element : elements_) {
            auto* button = dynamic_cast<UiButton*>(element);
            if (button) {
                bool prevHover = button->isHovered();
                button->setHovered(button->containsPoint(point));
                // Mark dirty if hover state changed (active style index changed)
                if (prevHover != button->isHovered()) {
                    markDirty(button->getElementId());
                }
            }
        }
    }

    void UiWrapper::onMouseButton(int button, int action, int mods, double x, double y) {
        (void)mods;
        if (!engine_ || button != 0 || action != 1) return;

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
            // Clear dirty flags
            for (uint32_t i = firstDirty_; i < lastDirty_; i++) {
                styleDirty_[i] = false;
            }
            firstDirty_ = UiRenderer::MAX_ELEMENTS;
            lastDirty_ = 0;
        }

        for (auto* element : elements_) {
            element->render(*engine_);
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
        // Update elementStyles_ with the element's current active style index
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

} // namespace lve
