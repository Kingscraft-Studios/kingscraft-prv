#pragma once

#include "UI/Elements/UiElement.hpp"
#include <string>

namespace lve {

    class UiTextBlock : public UiElement {
    public:
        UiTextBlock() = default;
        ~UiTextBlock() override = default;

        UiTextBlock(const UiTextBlock&) = delete;
        UiTextBlock& operator=(const UiTextBlock&) = delete;

        void render(UiEngine& engine) override;

        void setText(const std::string& text) { text_ = text; }
        void setFont(const std::string& fontName) { fontName_ = fontName; }
        void setFontSize(float size) { fontSize_ = size; }

        const std::string& getText() const { return text_; }
        const std::string& getFont() const { return fontName_; }
        float getFontSize() const { return fontSize_; }

        float getTextWidth(UiEngine& engine) const;

    private:
        std::string text_;
        std::string fontName_;
        float fontSize_ = 24.0f;
    };

} // namespace lve
