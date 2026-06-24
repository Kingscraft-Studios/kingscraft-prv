#include "UI/UiWrapper.hpp"

namespace lve {

    UiWrapper::~UiWrapper() {

    }

    void UiWrapper::init() {

    }

    void UiWrapper::shutdown() {

    }

    void UiWrapper::update(double deltaTime) {

    }

    void UiWrapper::resize(int width, int height) {

    }

    void UiWrapper::onMouseMove(double x, double y) {

    }

    void UiWrapper::onMouseButton(int button, int action, int mods, double x, double y) {

    }

    void UiWrapper::onScroll(double dx, double dy) {

    }

    void UiWrapper::onKey(int key, int action) {

    }

    void UiWrapper::onChar(unsigned int codepoint) {

    }

    void UiWrapper::renderOffscreen(VkCommandBuffer cmdBuffer) {

    }

    void UiWrapper::render(VkCommandBuffer cmdBuffer, VkRenderPass renderPass) {

    }

    void UiWrapper::registerButtonHandler(std::string name, std::function<void()> handler) {

    }

}
