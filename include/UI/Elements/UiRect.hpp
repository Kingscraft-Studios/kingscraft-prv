#pragma once

#include "UI/Elements/UiElement.hpp"

namespace lve {

    class UiRect : public UiElement {
    public:
        UiRect() = default;
        ~UiRect() override = default;

        UiRect(const UiRect&) = delete;
        UiRect& operator=(const UiRect&) = delete;

        void render(UiEngine& engine) override;
    };

} // namespace lve
