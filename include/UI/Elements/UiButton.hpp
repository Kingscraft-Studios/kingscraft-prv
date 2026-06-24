#pragma once

#include "UI/Elements/UiElement.hpp"
#include <string>
#include <functional>

namespace lve {

    class UiButton : public UiElement {
    public:
        UiButton() = default;
        ~UiButton() override = default;

        UiButton(const UiButton&) = delete;
        UiButton& operator=(const UiButton&) = delete;

        void render(UiEngine& engine) override;
        bool containsPoint(glm::vec2 point) const override;

        void setText(const std::string& text) { text_ = text; }
        void setFont(const std::string& fontName) { fontName_ = fontName; }
        void setFontSize(float size) { fontSize_ = size; }
        void setOnClick(std::function<void()> callback) { onClick_ = std::move(callback); }
        void setHandlerName(const std::string& name) { handlerName_ = name; }

        const std::string& getText() const { return text_; }
        float getFontSize() const { return fontSize_; }
        const std::string& getHandlerName() const { return handlerName_; }

        void setHovered(bool hovered) { isHovered_ = hovered; }
        bool isHovered() const { return isHovered_; }

        void setNormalColor(glm::vec4 color) { normalColor_ = color; }
        void setHoverColor(glm::vec4 color) { hoverColor_ = color; }
        void setTextColor(glm::vec4 color) { textColor_ = color; }

        const glm::vec4& getNormalColor() const { return normalColor_; }
        const glm::vec4& getHoverColor() const { return hoverColor_; }
        const glm::vec4& getTextColor() const { return textColor_; }

        void click();

    private:
        std::string text_;
        std::string fontName_;
        std::string handlerName_;
        float fontSize_ = 24.0f;
        bool isHovered_ = false;

        glm::vec4 normalColor_{0.3f, 0.3f, 0.3f, 1.0f};
        glm::vec4 hoverColor_{0.5f, 0.5f, 0.5f, 1.0f};
        glm::vec4 textColor_{1.0f, 1.0f, 1.0f, 1.0f};

        std::function<void()> onClick_;
    };

} // namespace lve
