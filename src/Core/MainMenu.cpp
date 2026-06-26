// TODO: SSBO Style System — MainMenu redesign
// Uses registered styles with the SSBO system:
// - Radial gradient background via UiGradientRect
// - Gold text with Arial font
// - Buttons with transparent bg, gold text, gold selection bar
// - Hover styles for text color change (gold_dark → gold)
// - Selection bar rects as child elements alongside buttons

#include "Core/MainMenu.hpp"

#include "Core/World/WorldScreen.hpp"
#include "Core/Constants.hpp"
#include "Renderer/Renderer.hpp"

namespace lve {

    // Shared style indices for all MainMenu instances
    static uint32_t g_styleBg = 0;
    static uint32_t g_styleTitle = 0;
    static uint32_t g_styleSubtitle = 0;
    static uint32_t g_styleBtnText = 0;
    static uint32_t g_styleBtnTextHover = 0;
    static uint32_t g_styleSelBar = 0;
    static bool g_stylesInit = false;

    static void ensureStyles(UiWrapper& ui) {
        if (g_stylesInit) return;
        g_styleBg = ui.registerStyle(UiStyle{
            .mode = RenderMode::Solid,
            .color1 = BG_DARK,
        });
        g_styleTitle = ui.registerStyle(UiStyle{
            .mode = RenderMode::Font,
            .color1 = GOLD
        });
        g_styleSubtitle = ui.registerStyle(UiStyle{
            .mode = RenderMode::Font,
            .color1 = GOLD_DARK
        });
        g_styleBtnText = ui.registerStyle(UiStyle{
            .mode = RenderMode::Font,
            .color1 = GOLD_DARK
        });
        g_styleBtnTextHover = ui.registerStyle(UiStyle{
            .mode = RenderMode::Font,
            .color1 = GOLD
        });
        g_styleSelBar = ui.registerStyle(UiStyle{
            .mode = RenderMode::Solid,
            .color1 = GOLD
        });
        g_stylesInit = true;
    }

    MainMenu::MainMenu(VkRenderPass renderPass, UiWrapper& uiSystem, VkExtent2D extent)
        : renderPass_(renderPass), uiSystem_(uiSystem), extent_(extent) {
    }

    void MainMenu::init() {
        ensureStyles(uiSystem_);
        createBackground();
        createTitle();
        createButtons();
    }

    void MainMenu::cleanup() {
        uiSystem_.removeElement(&background_);
        uiSystem_.removeElement(&title_);
        uiSystem_.removeElement(&subtitle_);
        uiSystem_.removeElement(&selectionBarEnter_);
        uiSystem_.removeElement(&selectionBarQuit_);
        uiSystem_.removeElement(&enterWorldButton_);
        uiSystem_.removeElement(&quitButton_);
    }

    void MainMenu::onSwapChainRecreated(VkExtent2D extent) {
        extent_ = extent;
    }

    void MainMenu::createBackground() {
        background_.setPosition({0, 0});
        background_.setSize({static_cast<float>(extent_.width),
                             static_cast<float>(extent_.height)});
        background_.setStyleIndex(g_styleBg);
        background_.setName("BgGradient");
        uiSystem_.addElement(&background_);
    }

    void MainMenu::createTitle() {
        float cx = static_cast<float>(extent_.width) * 0.5f;

        title_.setPosition({cx - 300.0f, 100.0f});
        title_.setSize({600.0f, 70.0f});
        title_.setText("KINGSCRAFT");
        title_.setFont("default");
        title_.setFontSize(64.0f);
        title_.setColor(GOLD);
        title_.setStyleIndex(g_styleTitle);
        title_.setName("Title");
        uiSystem_.addElement(&title_);

        subtitle_.setPosition({cx - 200.0f, 170.0f});
        subtitle_.setSize({400.0f, 30.0f});
        subtitle_.setText("THE SANDBOX WITHOUT LIMITS");
        subtitle_.setFont("default");
        subtitle_.setFontSize(18.0f);
        subtitle_.setColor(GOLD_DARK);
        subtitle_.setStyleIndex(g_styleSubtitle);
        subtitle_.setName("Subtitle");
        uiSystem_.addElement(&subtitle_);
    }

    void MainMenu::createButtons() {
        float cx = 120.0f;  // left-aligned
        float bw = 300.0f;
        float bh = 50.0f;
        float spacing = 10.0f;
        float barW = 4.0f;
        float barH = 40.0f;
        float startY = static_cast<float>(extent_.height) * 0.45f;
        float barOffsetY = (bh - barH) * 0.5f;

        // "ENTER WORLD" button
        enterWorldButton_.setPosition({cx + 20.0f, startY});
        enterWorldButton_.setSize({bw, bh});
        enterWorldButton_.setText("ENTER WORLD");
        enterWorldButton_.setFont("default");
        enterWorldButton_.setFontSize(38.0f);
        enterWorldButton_.setNormalColor(TRANSPARENT);
        enterWorldButton_.setHoverColor(TRANSPARENT);
        enterWorldButton_.setNormalTextColor(GOLD_DARK);
        enterWorldButton_.setHoverTextColor(GOLD);
        enterWorldButton_.setStyleIndex(g_styleBtnText);
        enterWorldButton_.setHoverStyleIndex(g_styleBtnTextHover);
        enterWorldButton_.setHandlerName(BTN_ENTER_WORLD);
        enterWorldButton_.setName("EnterWorldButton");
        uiSystem_.addElement(&enterWorldButton_);

        // Selection bar for "ENTER WORLD"
        selectionBarEnter_.setPosition({cx, startY + barOffsetY});
        selectionBarEnter_.setSize({barW, barH});
        selectionBarEnter_.setColor(GOLD);
        selectionBarEnter_.setStyleIndex(g_styleSelBar);
        selectionBarEnter_.setName("SelBarEnter");
        uiSystem_.addElement(&selectionBarEnter_);

        // "QUIT GAME" button
        quitButton_.setPosition({cx + 20.0f, startY + bh + spacing});
        quitButton_.setSize({bw, bh});
        quitButton_.setText("QUIT GAME");
        quitButton_.setFont("default");
        quitButton_.setFontSize(38.0f);
        quitButton_.setNormalColor(TRANSPARENT);
        quitButton_.setHoverColor(TRANSPARENT);
        quitButton_.setNormalTextColor(GOLD_DARK);
        quitButton_.setHoverTextColor(GOLD);
        quitButton_.setStyleIndex(g_styleBtnText);
        quitButton_.setHoverStyleIndex(g_styleBtnTextHover);
        quitButton_.setHandlerName(BTN_QUIT_GAME);
        quitButton_.setName("QuitButton");
        uiSystem_.addElement(&quitButton_);

        // Selection bar for "QUIT GAME"
        selectionBarQuit_.setPosition({cx, startY + bh + spacing + barOffsetY});
        selectionBarQuit_.setSize({barW, barH});
        selectionBarQuit_.setColor(GOLD);
        selectionBarQuit_.setStyleIndex(g_styleSelBar);
        selectionBarQuit_.setName("SelBarQuit");
        uiSystem_.addElement(&selectionBarQuit_);
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
