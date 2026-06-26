#pragma once

#include <glm/glm.hpp>
#include <string>
#include <cstdint>

namespace lve {

    class UiEngine;

    class UiElement {
    public:
        UiElement() = default;
        virtual ~UiElement() = default;

        UiElement(const UiElement&) = delete;
        UiElement& operator=(const UiElement&) = delete;

        virtual void render(UiEngine& engine) = 0;
        virtual bool containsPoint(glm::vec2 point) const;

        void setPosition(glm::vec2 pos) { position_ = pos; }
        void setSize(glm::vec2 size) { size_ = size; }
        void setColor(glm::vec4 color) { color_ = color; }
        void setVisible(bool visible) { visible_ = visible; }
        void setName(const std::string& name) { name_ = name; }

        glm::vec2 getPosition() const { return position_; }
        glm::vec2 getSize() const { return size_; }
        glm::vec4 getColor() const { return color_; }
        bool isVisible() const { return visible_; }
        const std::string& getName() const { return name_; }

        void setElementId(uint32_t id) { elementId_ = id; }
        uint32_t getElementId() const { return elementId_; }

        void setStyleIndex(uint32_t index) { styleIndex_ = index; }
        uint32_t getStyleIndex() const { return styleIndex_; }
        virtual uint32_t getActiveStyleIndex() const { return styleIndex_; }

    protected:
        glm::vec2 position_{0.0f};
        glm::vec2 size_{0.0f};
        glm::vec4 color_{1.0f};
        bool visible_ = true;
        std::string name_;
        uint32_t elementId_ = 0;
        uint32_t styleIndex_ = 0;
    };

} // namespace lve
