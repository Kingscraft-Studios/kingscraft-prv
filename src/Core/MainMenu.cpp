#include "Core/MainMenu.hpp"

#include "Core/World/WorldScreen.hpp"
#include "Core/Constants.hpp"
#include "Renderer/Renderer.hpp"

namespace lve {

    MainMenu::MainMenu(VkRenderPass renderPass, UiWrapper& uiSystem, VkExtent2D extent)
        : renderPass_(renderPass), uiSystem_(uiSystem), extent_(extent) {
    }

    void MainMenu::init() {
        createTitle();
        createButtons();
    }

    void MainMenu::cleanup() {
        uiSystem_.removeElement(&title_);
        uiSystem_.removeElement(&enterWorldButton_);
        uiSystem_.removeElement(&quitButton_);
    }

    void MainMenu::onSwapChainRecreated(VkExtent2D extent) {
        extent_ = extent;
    }

    void MainMenu::createTitle() {
        float cx = static_cast<float>(extent_.width) * 0.5f;
        float titleW = 400.0f;
        float titleH = 60.0f;

        title_.setPosition({cx - titleW * 0.5f, 80.0f});
        title_.setSize({titleW, titleH});
        title_.setText("Kingscraft");
        title_.setFont("default");
        title_.setFontSize(48.0f);
        title_.setColor({1.0f, 1.0f, 1.0f, 1.0f});
        title_.setName("Title");

        uiSystem_.addElement(&title_);
    }

    void MainMenu::createButtons() {
        float cx = static_cast<float>(extent_.width) * 0.5f;
        float bw = 260.0f;
        float bh = 50.0f;
        float spacing = 20.0f;
        float startY = static_cast<float>(extent_.height) * 0.45f;

        enterWorldButton_.setPosition({cx - bw * 0.5f, startY});
        enterWorldButton_.setSize({bw, bh});
        enterWorldButton_.setText("Enter World");
        enterWorldButton_.setFont("default");
        enterWorldButton_.setFontSize(24.0f);
        enterWorldButton_.setNormalColor({0.2f, 0.4f, 0.8f, 1.0f});
        enterWorldButton_.setHoverColor({0.3f, 0.6f, 1.0f, 1.0f});
        enterWorldButton_.setTextColor({1.0f, 1.0f, 1.0f, 1.0f});
        enterWorldButton_.setHandlerName(BTN_ENTER_WORLD);
        enterWorldButton_.setName("EnterWorldButton");

        uiSystem_.addElement(&enterWorldButton_);

        quitButton_.setPosition({cx - bw * 0.5f, startY + bh + spacing});
        quitButton_.setSize({bw, bh});
        quitButton_.setText("Quit");
        quitButton_.setFont("default");
        quitButton_.setFontSize(24.0f);
        quitButton_.setNormalColor({0.6f, 0.2f, 0.2f, 1.0f});
        quitButton_.setHoverColor({0.9f, 0.3f, 0.3f, 1.0f});
        quitButton_.setTextColor({1.0f, 1.0f, 1.0f, 1.0f});
        quitButton_.setHandlerName(BTN_QUIT_GAME);
        quitButton_.setName("QuitButton");

        uiSystem_.addElement(&quitButton_);
    }

    FrameRenderInfo MainMenu::getFrameRenderInfo(const Renderer& renderer, uint32_t) const {
        return {
            renderPass_,
            renderer.getCurrentFramebuffer(),
            {{{0.1f, 0.1f, 0.1f, 1.0f}}},
            true
        };
    }
} // namespace lve
