#pragma once

#include "UI/Elements/UiElement.hpp"

namespace lve {

    class UiGradientRect : public UiElement {
    public:
        UiGradientRect() = default;
        ~UiGradientRect() override = default;

        UiGradientRect(const UiGradientRect&) = delete;
        UiGradientRect& operator=(const UiGradientRect&) = delete;

        void render(UiEngine& engine) override;
    };

} // namespace lve
