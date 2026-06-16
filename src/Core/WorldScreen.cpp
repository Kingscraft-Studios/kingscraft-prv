#include "Core/WorldScreen.hpp"

namespace lve {

    WorldScreen::WorldScreen(Device& device)
        : device_(device) {}

    WorldScreen::~WorldScreen() {
        cleanup();
    }

    void WorldScreen::init() {}

    void WorldScreen::tick(double dt) {}

    void WorldScreen::render(const FrameContext& ctx) {}

    void WorldScreen::cleanup() {}

    void WorldScreen::onRenderPassChanged(VkRenderPass renderPass) {}

} // namespace lve
