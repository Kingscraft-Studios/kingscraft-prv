#include "Core/MainMenu.hpp"

#include "Core/WorldScreen.hpp"
#include "Renderer/Renderer.hpp"

namespace lve {

    MainMenu::MainMenu(VkRenderPass renderPass, UiSystem& uiSystem)
        : renderPass_(renderPass), uiSystem_(uiSystem) {}

    FrameRenderInfo MainMenu::getFrameRenderInfo(const Renderer& renderer, uint32_t) const {
        return {
            renderPass_,
            renderer.getCurrentFramebuffer(),
            {{{0.1f, 0.1f, 0.1f, 1.0f}}},
            true
        };
    }
} // namespace lve
