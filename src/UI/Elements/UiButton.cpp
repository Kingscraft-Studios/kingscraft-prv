#include "UI/Elements/UiButton.hpp"
#include "UI/Engine/UiEngine.hpp"
#include "UI/Engine/UiFontAtlas.hpp"

namespace lve {

    void UiButton::render(UiEngine& engine) {
        if (!visible_) return;

        // Background
        glm::vec4 bgColor = isHovered_ ? hoverColor_ : normalColor_;
        float x0 = position_.x;
        float y0 = position_.y;
        float x1 = position_.x + size_.x;
        float y1 = position_.y + size_.y;

        UiVertex bgVerts[4] = {
            {{x0, y0}, {-1.0f, -1.0f}, bgColor},
            {{x1, y0}, {-1.0f, -1.0f}, bgColor},
            {{x1, y1}, {-1.0f, -1.0f}, bgColor},
            {{x0, y1}, {-1.0f, -1.0f}, bgColor},
        };
        uint32_t indices[6] = {0, 1, 2, 0, 2, 3};
        engine.addQuad(bgVerts, indices);

        // Text
        if (text_.empty()) return;

        float scale = fontSize_ / UiFontAtlas::ATLAS_FONT_SIZE;
        float textW = 0.0f;
        for (char32_t c : text_) {
            const auto* g = engine.getGlyph(fontName_, c);
            if (g) textW += g->advance * scale;
        }

        float penX = position_.x + (size_.x - textW) * 0.5f;
        float penY = position_.y + size_.y * 0.5f + fontSize_ * 0.35f;

        for (char32_t c : text_) {
            const auto* g = engine.getGlyph(fontName_, c);
            if (!g) continue;

            float gx0 = penX + g->bearingX * scale;
            float gy0 = penY + g->bearingY * scale;
            float gx1 = gx0 + g->width * scale;
            float gy1 = penY + (g->bearingY + g->height) * scale;

            UiVertex textVerts[4] = {
                {{gx0, gy0}, {g->uv0.x, g->uv0.y}, textColor_},
                {{gx1, gy0}, {g->uv1.x, g->uv0.y}, textColor_},
                {{gx1, gy1}, {g->uv1.x, g->uv1.y}, textColor_},
                {{gx0, gy1}, {g->uv0.x, g->uv1.y}, textColor_},
            };
            engine.addQuad(textVerts, indices);

            penX += g->advance * scale;
        }
    }

    bool UiButton::containsPoint(glm::vec2 point) const {
        if (!visible_) return false;
        return point.x >= position_.x && point.x <= position_.x + size_.x &&
               point.y >= position_.y && point.y <= position_.y + size_.y;
    }

    void UiButton::click() {
        if (onClick_) onClick_();
    }

} // namespace lve
