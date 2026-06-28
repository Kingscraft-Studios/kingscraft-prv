#include "UI/Debug/UiFpsCounter.hpp"
#include "UI/UiWrapper.hpp"
#include "UI/Engine/UiStyle.hpp"

namespace lve {

    void UiFpsCounter::init(UiWrapper& ui) {
        styleIndex_ = ui.registerStyle(UiStyle{
            .mode = RenderMode::Font,
            .color1 = {1.0f, 1.0f, 1.0f, 1.0f}
        });

        text_.setAnchor({0.0f, 0.0f}, {10.0f, 10.0f});
        text_.setNormalizedSize({0.3f, 0.04f});
        text_.setText("FPS: --  Frame: --  CPU: --  GPU: --");
        text_.setFont("default");
        text_.setColor({1.0f, 1.0f, 1.0f, 1.0f});
        text_.setStyleIndex(styleIndex_);
        text_.setName("FpsCounter");
        ui.addElement(&text_);

        lastTime_ = TimeUtil::uptimeSeconds();
    }

    void UiFpsCounter::update() {
        double now = TimeUtil::uptimeSeconds();
        double dt = now - lastTime_;
        lastTime_ = now;

        elapsed_ += dt;
        frameCount_++;

        if (elapsed_ >= 0.25) {
            float fps = frameCount_ / static_cast<float>(elapsed_);
            float ms = (elapsed_ / static_cast<double>(frameCount_)) * 1000.0f;

            char buf[96];
            int n = snprintf(buf, sizeof(buf),
                "FPS: %.0f  Frame: %.1fms  CPU: %.1fms  GPU: %.1fms",
                fps, ms, latestCpuMs_, latestGpuMs_);
            text_.setText(std::string(buf, n));

            elapsed_ = 0.0;
            frameCount_ = 0;
        }
    }

    void UiFpsCounter::setCpuGpuTimes(double cpuMs, double gpuMs) {
        latestCpuMs_ = cpuMs;
        latestGpuMs_ = gpuMs;
    }

    void UiFpsCounter::cleanup(UiWrapper& ui) {
        ui.removeElement(&text_);
    }

} // namespace lve
