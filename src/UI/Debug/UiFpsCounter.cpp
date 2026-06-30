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
        text_.setNormalizedSize({0.5f, 0.06f});
        text_.setFontSize(28.0f);
        text_.setText("FPS: --  Frame: --  GPU: --  Terrain: --");
        text_.setFont("default");
        text_.setColor({1.0f, 1.0f, 1.0f, 1.0f});
        text_.setStyleIndex(styleIndex_);
        text_.setName("FpsCounter");
        ui.addElement(&text_);

        cpuBreakdown_.setAnchor({0.0f, 0.03f}, {10.0f, 10.0f});
        cpuBreakdown_.setNormalizedSize({0.5f, 0.04f});
        cpuBreakdown_.setFontSize(18.0f);
        cpuBreakdown_.setText("CPU: --  Tick: --  Frustum: --  Draw: --  Sub: --");
        cpuBreakdown_.setFont("default");
        cpuBreakdown_.setColor({0.7f, 0.7f, 1.0f, 1.0f});
        cpuBreakdown_.setStyleIndex(styleIndex_);
        cpuBreakdown_.setName("CpuBreakdown");
        ui.addElement(&cpuBreakdown_);

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

            char buf[128];
            int n = snprintf(buf, sizeof(buf),
                "FPS: %.0f  Frame: %.1fms  GPU: %.1fms  Terrain: %.1fms",
                fps, ms, latestGpuMs_, latestTerrainMs_);
            text_.setText(std::string(buf, n));

            char brk[96];
            int nb = snprintf(brk, sizeof(brk),
                "CPU: %.1fms  Tick: %.2f  Frustum: %.2f  Draw: %.1f  Sub: %.2f",
                latestCpuMs_ + latestCpuTickMs_ + latestCpuSubmitMs_,
                latestCpuTickMs_, latestFrustumMs_, latestDrawMs_, latestCpuSubmitMs_);
            cpuBreakdown_.setText(std::string(brk, nb));

            elapsed_ = 0.0;
            frameCount_ = 0;
        }
    }

    void UiFpsCounter::setCpuGpuTimes(double cpuMs, double gpuMs, double terrainMs,
                                       double cpuTickMs, double cpuSubmitMs,
                                       double frustumMs, double drawMs) {
        latestCpuMs_ = cpuMs;
        latestGpuMs_ = gpuMs;
        latestTerrainMs_ = terrainMs;
        latestCpuTickMs_ = cpuTickMs;
        latestCpuSubmitMs_ = cpuSubmitMs;
        latestFrustumMs_ = frustumMs;
        latestDrawMs_ = drawMs;
    }

    void UiFpsCounter::cleanup(UiWrapper& ui) {
        ui.removeElement(&text_);
        ui.removeElement(&cpuBreakdown_);
    }

} // namespace lve
