#include "UI/Elements/UiElement.hpp"

namespace lve {

    bool UiElement::containsPoint(glm::vec2 point) const {
        if (!visible_) return false;
        return point.x >= position_.x && point.x <= position_.x + size_.x &&
               point.y >= position_.y && point.y <= position_.y + size_.y;
    }

} // namespace lve
