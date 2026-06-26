#include "UI/Elements/UiTextBlock.hpp"
#include "UI/Engine/UiEngine.hpp"
#include "UI/Engine/UiFontAtlas.hpp"

namespace lve {

    void UiTextBlock::render(UiEngine& engine) {
        if (!visible_ || text_.empty()) return;

        float scale = fontSize_ / UiFontAtlas::ATLAS_FONT_SIZE;
        float penX = position_.x;
        float penY = position_.y + fontSize_;

        for (char32_t c : text_) {
            const auto* glyph = engine.getGlyph(fontName_, c);
            if (!glyph) continue;

            float x0 = penX + glyph->bearingX * scale;
            float y0 = penY + glyph->bearingY * scale;
            float x1 = x0 + glyph->width * scale;
            float y1 = penY + (glyph->bearingY + glyph->height) * scale;

            UiVertex verts[4] = {
                {{x0, y0}, {glyph->uv0.x, glyph->uv0.y}, color_, elementId_},
                {{x1, y0}, {glyph->uv1.x, glyph->uv0.y}, color_, elementId_},
                {{x1, y1}, {glyph->uv1.x, glyph->uv1.y}, color_, elementId_},
                {{x0, y1}, {glyph->uv0.x, glyph->uv1.y}, color_, elementId_},
            };

            uint32_t indices[6] = {0, 1, 2, 0, 2, 3};
            engine.addQuad(verts, indices);

            penX += glyph->advance * scale;
        }
    }

    float UiTextBlock::getTextWidth(UiEngine& engine) const {
        float scale = fontSize_ / UiFontAtlas::ATLAS_FONT_SIZE;
        float width = 0.0f;
        for (char32_t c : text_) {
            const auto* glyph = engine.getGlyph(fontName_, c);
            if (glyph) {
                width += glyph->advance * scale;
            }
        }
        return width;
    }

} // namespace lve
